pragma once

include <Arduino.h>

include "types.h"

struct ImuCalib {
    float accelBias[3];
    float gyroBias[3];
};

struct MagCalib {
    float offset[3];
    float scale[3];
};

struct CalibData {
    ImuCalib icm;
    ImuCalib gy91;
    MagCalib magGy91;
    MagCalib magGps;
    bool     valid;
};

bool calibLoad(CalibData& c);
bool calibSave(const CalibData& c);

// runtime application
void applyImuCalib(ImuSample& imu, const ImuCalib& c);
void applyMagCalib(ImuSample& imu, const MagCalib& c);
