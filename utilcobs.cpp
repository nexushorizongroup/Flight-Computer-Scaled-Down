include "util_cobs.h"

sizet cobsEncode(const uint8t src, sizet len, uint8t dst, size_t dstMax) {
    if (dstMax < len + 2) return 0;
    uint8_t* start = dst;
    uint8t* codeptr = dst++;
    uint8_t code = 1;
    for (size_t i = 0; i < len; i++) {
        if (src[i] == 0) {
            *code_ptr = code;
            code_ptr = dst++;
            code = 1;
        } else {
            *dst++ = src[i];
            code++;
            if (code == 0xFF) {
                *code_ptr = code;
                code_ptr = dst++;
                code = 1;
            }
        }
    }
    *code_ptr = code;
    *dst++ = 0x00;
    return dst - start;
}

sizet cobsDecode(const uint8t src, sizet len, uint8t dst, size_t dstMax) {
    if (len == 0) return 0;
    const uint8_t* end = src + len;
    size_t outLen = 0;
    while (src < end) {
        uint8_t code = *src++;
        if (code == 0 || src + code - 1 > end) return 0;
        for (uint8_t i = 1; i < code; i++) {
            if (outLen >= dstMax) return 0;
            dst[outLen++] = *src++;
        }
        if (code < 0xFF && src < end) {
            if (outLen >= dstMax) return 0;
            dst[outLen++] = 0x00;
        }
    }
    return outLen;
}
