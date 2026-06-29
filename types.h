pragma once

include <Arduino.h>

struct ImuSample {
    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;
    float qw, qx, qy, qz;
    float tempC;
    bool  valid;
};

struct BaroSample {
    float pressurePa;
    float temperatureC;
    bool  valid;
};

struct GpsSample {
    bool  valid;
    uint32_t towMs;
    double latDeg;
    double lonDeg;
    float altM;
    float velN;
    float velE;
    float velD;
    float hAcc;
    float vAcc;
    uint8_t fixType;
    uint8_t sats;
};

struct PowerSample {
    float vBatt;
    float v5;
    float v3v3;
    bool  valid;
};

struct RtcSample {
    uint32_t unixTime;
    bool     valid;
};

struct MagSample {
    float mx, my, mz;
    bool  valid;
};

enum FlightPhase : uint8_t {
    PHASE_IDLE = 0,
    PHASE_ARMED,
    PHASE_BOOST,
    PHASE_COAST,
    PHASE_DESCENT,
    PHASE_LANDED
};

struct SensorSnapshot {
    uint64_t tUs;
    ImuSample   imuPrimary;
    ImuSample   imuBackup;
    BaroSample  baroPrimary;
    BaroSample  baroBackup;
    GpsSample   gps;
    PowerSample power;
    RtcSample   rtc;
    MagSample   gpsMag;
};

struct FusionState {
    uint64_t tUs;

    float alt;
    float vel;
    float accelBias;

    float baroAlt;
    float gpsAlt;
    float gpsVelD;

    float gpsQuality;
    bool  gpsValid;

    bool  ekfInconsistent;
    bool  gpsTimeout;
    bool  gpsAccuracyBad;

    bool  pyroMainArmed;
    bool  pyroMainFired;
    bool  pyroDrogueArmed;
    bool  pyroDrogueFired;

    PowerSample power;

    bool  vibHigh;
    bool  machLock;
    bool  padTiltBad;

    FlightPhase phase;
};
