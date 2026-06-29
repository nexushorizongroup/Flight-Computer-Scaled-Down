pragma once

include <Arduino.h>

include "hardware_pins.h"

bool loraInit();
bool loraSend(const uint8t* data, sizet len);
