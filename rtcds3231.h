pragma once

include <Arduino.h>

include "types.h"

bool rtcInit();
bool rtcRead(RtcSample& out);
