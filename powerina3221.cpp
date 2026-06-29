include "power_ina3221.h"

include "i2cmuxtca9548a.h"

include <Wire.h>

static const uint8t INA3221ADDR = 0x40;

static uint16t readReg16(uint8t reg) {
    Wire.beginTransmission(INA3221_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return 0;
    Wire.requestFrom(INA3221ADDR, (uint8t)2);
    if (Wire.available() < 2) return 0;
    uint16_t msb = Wire.read();
    uint16_t lsb = Wire.read();
    return (msb << 8) | lsb;
}

void ina3221Init() {
    tcaSelect(TCA_INA3221);
}

bool ina3221Read(PowerSample& out) {
    if (!tcaSelect(TCA_INA3221)) return false;
    uint16_t v1 = readReg16(0x02);
    uint16_t v2 = readReg16(0x04);
    uint16_t v3 = readReg16(0x06);
    out.vBatt = v1 * 0.008f;
    out.v5    = v2 * 0.008f;
    out.v3v3  = v3 * 0.008f;
    out.valid = true;
    return true;
}
