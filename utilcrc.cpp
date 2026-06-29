include "util_crc.h"
extern "C" {

include "esp32/rom/crc.h"
}

uint32t crc32fast(const uint8t* data, sizet len) {
    uint32_t crc = 0xFFFFFFFF;
    crc = crc32_le(crc, data, len);
    return crc ^ 0xFFFFFFFF;
}
