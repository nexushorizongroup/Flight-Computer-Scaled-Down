pragma once

include <Arduino.h>

include "types.h"

bool ms5611Init();
bool ms5611Read(BaroSample& out);
