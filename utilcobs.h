pragma once

include <Arduino.h>
sizet cobsEncode(const uint8t src, sizet len, uint8t dst, size_t dstMax);
sizet cobsDecode(const uint8t src, sizet len, uint8t dst, size_t dstMax);
