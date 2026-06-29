include "imu_icm20948.h"

include "i2cmuxtca9548a.h"

include <SparkFunICM20948_ArduinoLibrary.h>

static ICM20948I2C icm;

bool icm20948Init() {
    tcaSelect(TCAIMUICM20948);
    if (icm.begin(Wire, ICM20948I2CADDRAD0) != ICM20948Stat_Ok) return false;
    if (icm.initializeDMP() != ICM20948Stat_Ok) return false;

    icm.enableDMPSensor(INVICM20948SENSOR_ACCELEROMETER);
    icm.enableDMPSensor(INVICM20948SENSOR_GYROSCOPE);
    icm.enableDMPSensor(INVICM20948SENSOR_MAGNETOMETER);
    icm.enableDMPSensor(INVICM20948SENSOR_ORIENTATION);

    icm.setDMPODRrate(DMPODRReg_Accel, 0);
    icm.setDMPODRrate(DMPODRReg_Gyro, 0);
    icm.setDMPODRrate(DMPODRReg_Compass, 0);
    icm.setDMPODRrate(DMPODRReg_Quat6, 0);

    icm.enableFIFO();
    icm.resetFIFO();
    return true;
}

bool icm20948Read(ImuSample& out) {
    tcaSelect(TCAIMUICM20948);
    icm.readDMPdataFromFIFO();
    if (icm.status != ICM20948Stat_Ok) {
        out.valid = false;
        return false;
    }
    if (icm.dmpData.quat6DataIsValid) {
        out.qw = icm.dmpData.quat6.q1;
        out.qx = icm.dmpData.quat6.q2;
        out.qy = icm.dmpData.quat6.q3;
        out.qz = icm.dmpData.quat6.q4;
    }
    if (icm.dmpData.accelDataIsValid) {
        out.ax = icm.dmpData.accel.accelX;
        out.ay = icm.dmpData.accel.accelY;
        out.az = icm.dmpData.accel.accelZ;
    }
    if (icm.dmpData.gyroDataIsValid) {
        out.gx = icm.dmpData.gyro.gyroX;
        out.gy = icm.dmpData.gyro.gyroY;
        out.gz = icm.dmpData.gyro.gyroZ;
    }
    if (icm.dmpData.compassDataIsValid) {
        out.mx = icm.dmpData.compass.CompassX;
        out.my = icm.dmpData.compass.CompassY;
        out.mz = icm.dmpData.compass.CompassZ;
    }
    out.tempC = icm.temp();
    out.valid = true;
    return true;
}
