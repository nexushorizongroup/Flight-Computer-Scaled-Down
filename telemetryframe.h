pragma once

include <Arduino.h>

include "types.h"

enum TelemetryType : uint8_t {
    TLMTYPEFUSION_STATE = 1,
};

struct TelemetryHeader {
    uint8_t  type;
    uint8_t  version;
    uint16_t length;
    uint32_t crc;
} attribute((packed));

size_t buildFusionTelemetry(const FusionState& state,
                            uint8t* outBuf, sizet outMax);
