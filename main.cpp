#include <Arduino.h>

#include <Wire.h>

#include <SPI.h>

#include "hardware_pins.h"

#include "types.h"

#include "ring_buffer.h"

#include "i2cmuxtca9548a.h"

#include "power_ina3221.h"

#include "imu_icm20948.h"

#include "imu_gy91.h"

#include "baro_ms5611.h"

#include "rtc_ds3231.h"

#include "gpsm100ubx.h"

#include "lora_llcc68.h"

#include "sd_logger.h"

#include "pyro_lr7843.h"

#include "ekf_vertical.h"

#include "telemetry_frame.h"


#include "calibration.h"

static QueueHandle_t qSensor = nullptr;
static QueueHandle_t qFusion = nullptr;
static QueueHandle_t qPyroCmd = nullptr;

static CalibData gCalib{};

void sensorTask(void* pvParameters);
void fusionTask(void* pvParameters);
void loggingTask(void* pvParameters);
void radioTask(void* pvParameters);
void pyroTask(void* pvParameters);

void setup() {
    Serial.begin(115200);
    delay(500);

    Wire.begin(PINI2CSDA, PINI2CSCL);
    tcaInit();
    pyroInit();

    calibLoad(gCalib); // if not valid, gCalib.valid=false

    qSensor = xQueueCreate(16, sizeof(SensorSnapshot));
    qFusion = xQueueCreate(16, sizeof(FusionState));
    qPyroCmd = xQueueCreate(8, sizeof(uint8_t));

    xTaskCreatePinnedToCore(sensorTask,"sensorTask",8192,nullptr,3,nullptr,0);
    xTaskCreatePinnedToCore(fusionTask,"fusionTask",8192,nullptr,4,nullptr,1);
    xTaskCreatePinnedToCore(loggingTask,"loggingTask",8192,nullptr,2,nullptr,0);
    xTaskCreatePinnedToCore(radioTask,"radioTask",4096,nullptr,2,nullptr,1);
    xTaskCreatePinnedToCore(pyroTask,"pyroTask",4096,nullptr,4,nullptr,0);
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}

static float verticalAccelFromImu(const ImuSample& imu) {
    if (!imu.valid) return 0.0f;
    float qw = imu.qw, qx = imu.qx, qy = imu.qy, qz = imu.qz;
    float ax = imu.ax, ay = imu.ay, az = imu.az;
    float R20 = 2.0f * (qx * qz - qy * qw);
    float R21 = 2.0f * (qy * qz + qx * qw);
    float R22 = 1.0f - 2.0f * (qx * qx + qy * qy);
    float awz = R20 * ax + R21 * ay + R22 * az;
    const float g = 9.80665f;
    return awz - g;
}

static float tiltFromImu(const ImuSample& imu) {
    if (!imu.valid) return 0.0f;
    float qw = imu.qw, qx = imu.qx, qy = imu.qy, qz = imu.qz;
    float R22 = 1.0f - 2.0f * (qx * qx + qy * qy);
    float cosTheta = constrain(R22, -1.0f, 1.0f);
    return acosf(cosTheta) * 180.0f / PI;
}

static void updateVibration(const ImuSample& imuBackup, FusionState& state) {
    if (!imuBackup.valid) { state.vibHigh=false; return; }
    static float buf[32]; static int idx=0; static bool filled=false;
    float a = sqrtf(imuBackup.ax * imuBackup.ax + imuBackup.ay * imuBackup.ay + imuBackup.az * imuBackup.az);
    buf[idx++]=a; if (idx>=32){idx=0;filled=true;}
    int n = filled?32:idx;
    if (n<4){state.vibHigh=false;return;}
    float mean=0; for(int i=0;i<n;i++)mean+=buf[i]; mean/=n;
    float var=0; for(int i=0;i<n;i++){float d=buf[i]-mean;var+=d*d;} var/=n;
    float rms = sqrtf(var);
    state.vibHigh = (rms>1.5f);
}

static void updateMachLock(const FusionState& in, FusionState& out) {
    static int lockCount = 0, unlockCount = 0;
    bool mach = out.machLock;
    bool highVel = fabs(in.vel)>150.0f;
    bool midAlt  = in.alt>300.0f && in.alt<3000.0f;
    bool vib     = in.vibHigh;
    if (highVel && midAlt && vib) {
        lockCount++;
        if (lockCount > 3) { mach = true; unlockCount = 0; }
    } else {
        unlockCount++;
        if (unlockCount > 3) { mach = false; lockCount = 0; }
    }
    out.machLock = mach;
}

static void updatePhase(FusionState& s) {
    // Clear, non-overlapping transitions
    if (fabs(s.vel)<1.0f && s.alt<5.0f && !s.pyroMainFired) s.phase=PHASE_IDLE;
    else if (s.vel>5.0f && s.alt<30.0f && !s.pyroMainFired) s.phase=PHASE_BOOST;
    else if (s.vel>0.0f && !s.pyroMainFired && s.alt>=30.0f) s.phase=PHASE_COAST;
    else if (s.vel<-5.0f && !s.pyroMainFired) s.phase=PHASE_DESCENT;
    else if (fabs(s.vel)<1.0f && s.alt<20.0f && s.pyroMainFired) s.phase=PHASE_LANDED;
}


void sensorTask(void* pvParameters) {
    ina3221Init();
    icm20948Init();
    gy91Init();
    ms5611Init();
    rtcInit();
    gpsInit(Serial1);
    sdInit();
    loraInit();

    const TickType_t period = pdMS_TO_TICKS(5);
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        vTaskDelayUntil(&lastWake, period);

        SensorSnapshot snap{};
        snap.tUs = micros();

        icm20948Read(snap.imuPrimary);
        gy91Read(snap.imuBackup, snap.baroBackup);
        ms5611Read(snap.baroPrimary);
        ina3221Read(snap.power);
        rtcRead(snap.rtc);
        gpsUpdate();
        gpsGet(snap.gps);

        if (gCalib.valid) {
            applyImuCalib(snap.imuPrimary, gCalib.icm);
            applyImuCalib(snap.imuBackup, gCalib.gy91);
            applyMagCalib(snap.imuBackup, gCalib.magGy91);
        }

        xQueueOverwrite(qSensor, &snap);
    }
}

void fusionTask(void* pvParameters) {
    EkfState ekf; ekfInit(ekf);
    FusionState state{};
    SensorSnapshot snap{};
    uint64_t lastUs = micros();
    const TickType_t period = pdMS_TO_TICKS(5);
    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        vTaskDelayUntil(&lastWake, period);
        if (xQueueReceive(qSensor,&snap,0)==pdTRUE) {
            uint64_t nowUs = snap.tUs;
            float dt = (nowUs-lastUs)*1e-6f;
            lastUs = nowUs;

            float accPrimary = verticalAccelFromImu(snap.imuPrimary);
            float accBackup  = verticalAccelFromImu(snap.imuBackup);
            float accBlend = accPrimary;
            if (snap.imuPrimary.valid && snap.imuBackup.valid) {
                float diff=fabs(accPrimary-accBackup);
                if (diff>5.0f) accBlend=0.7f*accPrimary+0.3f*accBackup;
                else accBlend=0.9f*accPrimary+0.1f*accBackup;
            } else if (!snap.imuPrimary.valid && snap.imuBackup.valid) {
                accBlend=accBackup;
            }

            ekfPredict(ekf, accBlend, dt);

            float baroAlt=0.0f;
            if (snap.baroPrimary.valid) {
                baroAlt=(101325.0f-snap.baroPrimary.pressurePa)*0.083f;
                float R = state.machLock?20.0f:4.0f;
                ekfUpdateBaro(ekf,baroAlt,R);
            } else if (snap.baroBackup.valid) {
                baroAlt=(101325.0f-snap.baroBackup.pressurePa)*0.083f;
                ekfUpdateBaro(ekf,baroAlt,6.0f);
            }

            if (snap.gps.valid) {
                float gpsAlt = snap.gps.altM;
                float gpsVelD = snap.gps.velD;
                ekfUpdateGps(ekf,gpsAlt,gpsVelD,6.0f,4.0f);
                state.gpsValid=true;
                state.gpsAlt=gpsAlt;
                state.gpsVelD=gpsVelD;
            } else state.gpsValid=false;

            state.tUs = nowUs;
            state.alt = ekf.x[0];
            state.vel = ekf.x[1];
            state.accelBias = ekf.x[2];
            state.baroAlt = baroAlt;
            state.power = snap.power;

            float tiltDeg = tiltFromImu(snap.imuBackup);
            state.padTiltBad = (tiltDeg>10.0f);

            updateVibration(snap.imuBackup,state);

            updateMachLock(state,state);
            updatePhase(state);

            xQueueOverwrite(qFusion,&state);
        }
    }
}

void loggingTask(void* pvParameters) {
    FusionState state{};
    FusionState lastLogged{};
    while (true) {
        if (xQueueReceive(qFusion,&state,portMAX_DELAY)==pdTRUE) {
            if (memcmp(&state, &lastLogged, sizeof(FusionState)) != 0) {
                if (state.machLock || state.vibHigh) {
                    sdLogBacklogPush(state);
                } else {
                    sdLogBacklogFlush();
                    sdLogFusion(state);
                }
                lastLogged = state;
            }
        }
    }
}

void radioTask(void* pvParameters) {
    FusionState state{};
    FusionState lastSent{};
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        if (xQueuePeek(qFusion,&state,0)==pdTRUE) {
            if (memcmp(&state, &lastSent, sizeof(FusionState)) != 0) {
                uint8_t buf[256];
                size_t len = buildFusionTelemetry(state,buf,sizeof(buf));
                if (len) loraSend(buf,len);
                lastSent = state;
            }
        }
        TickType_t period;
        switch (state.phase) {
            case PHASE_IDLE:
            case PHASE_LANDED: period=pdMS_TO_TICKS(500); break;
            case PHASE_BOOST:  period=pdMS_TO_TICKS(50);  break;
            case PHASE_COAST:
            case PHASE_DESCENT: period=pdMS_TO_TICKS(100); break;
            default: period=pdMS_TO_TICKS(200); break;
        }
        vTaskDelayUntil(&lastWake,period);
    }
}

void pyroTask(void* pvParameters) {
    PyroCommand cmd;
    while (true) {
        if (xQueueReceive(qPyroCmd,&cmd,portMAX_DELAY)==pdTRUE) {
            if (cmd==PYROCMDFIRE_MAIN) pyroFireMain(200);
            else if (cmd==PYROCMDFIRE_DROGUE) { /* kept disabled this flight */ }
        }
    }
}
