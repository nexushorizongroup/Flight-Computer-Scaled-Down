pragma once

include <Arduino.h>

include "types.h"

void gpsInit(HardwareSerial& port);
void gpsUpdate();
bool gpsGet(GpsSample& out);
