include "imu_gy91.h"

include "i2cmuxtca9548a.h"

include <Adafruit_BMP280.h>

include <MPU9250.h>

include <MadgwickAHRS.h>

static MPU9250 mpu;
static Adafruit_BMP280 bmp;
static Madgwick filter;

bool gy91Init() {
    tcaSelect(TCA_GY91);
    if (!bmp.begin(0x76)) return false;
    if (!mpu.setup(0x68)) return false;
    filter.begin(200.0f);
    return true;
}

bool gy91Read(ImuSample& imuOut, BaroSample& baroOut) {
    tcaSelect(TCA_GY91);
    if (!mpu.update()) {
        imuOut.valid = false;
    } else {
        float ax = mpu.getAccX();
        float ay = mpu.getAccY();
        float az = mpu.getAccZ();
        float gx = mpu.getGyroX() * DEGTORAD;
        float gy = mpu.getGyroY() * DEGTORAD;
        float gz = mpu.getGyroZ() * DEGTORAD;
        float mx = mpu.getMagX();
        float my = mpu.getMagY();
        float mz = mpu.getMagZ();
        filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);
        imuOut.ax = ax; imuOut.ay = ay; imuOut.az = az;
        imuOut.gx = gx; imuOut.gy = gy; imuOut.gz = gz;
        imuOut.mx = mx; imuOut.my = my; imuOut.mz = mz;
        imuOut.qw = filter.q0; imuOut.qx = filter.q1;
        imuOut.qy = filter.q2; imuOut.qz = filter.q3;
        imuOut.tempC = mpu.getTemperature();
        imuOut.valid = true;
    }
    baroOut.temperatureC = bmp.readTemperature();
    baroOut.pressurePa   = bmp.readPressure();
    baroOut.valid        = true;
    return imuOut.valid || baroOut.valid;
}
