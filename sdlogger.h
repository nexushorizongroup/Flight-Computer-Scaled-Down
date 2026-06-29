pragma once

include <Arduino.h>

include "types.h"

bool sdInit();
void sdLogFusion(const FusionState& state);
void sdLogBacklogPush(const FusionState& state);
void sdLogBacklogFlush();
