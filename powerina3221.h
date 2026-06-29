pragma once

include <Arduino.h>

include "types.h"

void ina3221Init();
bool ina3221Read(PowerSample& out);
