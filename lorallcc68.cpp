include "lora_llcc68.h"

include <RadioLib.h>

static SPIClass spiLora(VSPI);
static Module* loraModule = nullptr;
static LLCC68* lora = nullptr;

bool loraInit() {
    spiLora.begin(PINSPISCK, PINSPIMISO, PINSPIMOSI, -1);
    spiLora.setHwCs(false);
    loraModule = new Module(PINLORACS, PINLORADIO, PINLORARST, RADIOLIB_NC, spiLora);
    lora = new LLCC68(loraModule);
    int state = lora->begin(915.0);
    if (state != RADIOLIBERRNONE) return false;
    lora->setOutputPower(10);
    return true;
}

bool loraSend(const uint8t* data, sizet len) {
    if (!lora) return false;
    int state = lora->transmit((uint8_t*)data, len);
    return state == RADIOLIBERRNONE;
}
