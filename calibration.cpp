include "calibration.h"

include <Preferences.h>

static const char* NS = "calib";

bool calibLoad(CalibData& c) {
    Preferences prefs;
    if (!prefs.begin(NS, true)) return false;
    bool valid = prefs.getBool("valid", false);
    if (!valid) { prefs.end(); c.valid=false; return false; }

    prefs.getBytes("icm_acc", c.icm.accelBias, sizeof(c.icm.accelBias));
    prefs.getBytes("icm_gyro", c.icm.gyroBias, sizeof(c.icm.gyroBias));
    prefs.getBytes("gy_acc",  c.gy91.accelBias, sizeof(c.gy91.accelBias));
    prefs.getBytes("gy_gyro", c.gy91.gyroBias, sizeof(c.gy91.gyroBias));
    prefs.getBytes("maggoff", c.magGy91.offset, sizeof(c.magGy91.offset));
    prefs.getBytes("maggscl", c.magGy91.scale,  sizeof(c.magGy91.scale));
    prefs.getBytes("magpoff", c.magGps.offset,  sizeof(c.magGps.offset));
    prefs.getBytes("magpscl", c.magGps.scale,   sizeof(c.magGps.scale));
    prefs.end();
    c.valid = true;
    return true;
}

bool calibSave(const CalibData& c) {
    Preferences prefs;
    if (!prefs.begin(NS, false)) return false;
    prefs.putBytes("icm_acc", c.icm.accelBias, sizeof(c.icm.accelBias));
    prefs.putBytes("icm_gyro", c.icm.gyroBias, sizeof(c.icm.gyroBias));
    prefs.putBytes("gy_acc",  c.gy91.accelBias, sizeof(c.gy91.accelBias));
    prefs.putBytes("gy_gyro", c.gy91.gyroBias, sizeof(c.gy91.gyroBias));
    prefs.putBytes("maggoff", c.magGy91.offset, sizeof(c.magGy91.offset));
    prefs.putBytes("maggscl", c.magGy91.scale,  sizeof(c.magGy91.scale));
    prefs.putBytes("magpoff", c.magGps.offset,  sizeof(c.magGps.offset));
    prefs.putBytes("magpscl", c.magGps.scale,   sizeof(c.magGps.scale));
    prefs.putBool("valid", true);
    prefs.end();
    return true;
}

void applyImuCalib(ImuSample& imu, const ImuCalib& c) {
    imu.ax -= c.accelBias[0];
    imu.ay -= c.accelBias[1];
    imu.az -= c.accelBias[2];
    imu.gx -= c.gyroBias[0];
    imu.gy -= c.gyroBias[1];
    imu.gz -= c.gyroBias[2];
}

void applyMagCalib(ImuSample& imu, const MagCalib& c) {
    imu.mx = (imu.mx - c.offset[0]) * c.scale[0];
    imu.my = (imu.my - c.offset[1]) * c.scale[1];
    imu.mz = (imu.mz - c.offset[2]) * c.scale[2];
}
