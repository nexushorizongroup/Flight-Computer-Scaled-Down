pragma once

include <Arduino.h>

include "hardware_pins.h"

enum PyroCommand : uint8_t {
    PYROCMDNONE = 0,
    PYROCMDFIRE_MAIN,
    PYROCMDFIRE_DROGUE
};

void pyroInit();
void pyroFireMain(uint32_t ms);
void pyroFireDrogue(uint32_t ms);
