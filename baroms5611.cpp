include "baro_ms5611.h"

include "i2cmuxtca9548a.h"

include <Wire.h>

static const uint8t MSADDR = 0x77;
static uint16_t Cc[7];

static void writeCmd(uint8_t cmd) {
    Wire.beginTransmission(MS_ADDR);
    Wire.write(cmd);
    Wire.endTransmission();
}

static uint32_t readADC() {
    Wire.beginTransmission(MS_ADDR);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(MSADDR, (uint8t)3);
    uint32_t v = 0;
    v = (uint32_t)Wire.read() << 16;
    v |= (uint32_t)Wire.read() << 8;
    v |= Wire.read();
    return v;
}

bool ms5611Init() {
    tcaSelect(TCABAROMS5611);
    writeCmd(0x1E);
    delay(10);
    for (int i = 0; i < 6; i++) {
        Wire.beginTransmission(MS_ADDR);
        Wire.write(0xA2 + i * 2);
        Wire.endTransmission();
        Wire.requestFrom(MSADDR, (uint8t)2);
        Cc[i+1] = (Wire.read() << 8) | Wire.read();
    }
    return true;
}

bool ms5611Read(BaroSample& out) {
    tcaSelect(TCABAROMS5611);
    writeCmd(0x48);
    delay(10);
    uint32_t D1 = readADC();
    writeCmd(0x58);
    delay(10);
    uint32_t D2 = readADC();

    int32t dT = D2 - ((int32t)Cc[5] << 8);
    int32t TEMP = 2000 + ((int64t)dT * Cc[6]) / 8388608;
    int64t OFF  = ((int64t)Cc[2] << 16) + ((int64_t)dT * Cc[4]) / 128;
    int64t SENS = ((int64t)Cc[1] << 15) + ((int64_t)dT * Cc[3]) / 256;
    int32t P = (int32t)(((D1 * SENS) / 2097152 - OFF) / 32768);

    out.temperatureC = TEMP / 100.0f;
    out.pressurePa   = P;
    out.valid        = true;
    return true;
}
