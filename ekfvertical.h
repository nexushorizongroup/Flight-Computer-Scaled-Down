pragma once

include <Arduino.h>

include "types.h"

struct EkfState {
    float x[3];
    float P[3][3];
};

void ekfInit(EkfState& s);
void ekfPredict(EkfState& s, float accMeas, float dt);
void ekfUpdateBaro(EkfState& s, float altMeas, float R);
void ekfUpdateGps(EkfState& s, float altMeas, float velMeas, float Ralt, float Rvel);
