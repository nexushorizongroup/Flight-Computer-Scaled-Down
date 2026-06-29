pragma once

include <Arduino.h>

include "types.h"

bool icm20948Init();
bool icm20948Read(ImuSample& out);
