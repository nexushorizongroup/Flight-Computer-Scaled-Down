include "i2cmuxtca9548a.h"

void tcaInit() {
    Wire.begin(PINI2CSDA, PINI2CSCL);
}

bool tcaSelect(TcaBus bus) {
    Wire.beginTransmission(TCA9548A_ADDR);
    Wire.write(1 << staticcast<uint8t>(bus));
    return Wire.endTransmission() == 0;
}
