pragma once

include <Arduino.h>

include <Wire.h>

include "hardware_pins.h"

void tcaInit();
bool tcaSelect(TcaBus bus);
