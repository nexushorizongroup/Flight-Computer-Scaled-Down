pragma once

include <Arduino.h>

include "types.h"

bool gy91Init();
bool gy91Read(ImuSample& imuOut, BaroSample& baroOut);
