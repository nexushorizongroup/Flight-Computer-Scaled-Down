include "pyro_lr7843.h"

void pyroInit() {
    pinMode(PINPYROMAIN_GATE, OUTPUT);
    pinMode(PINPYRODROGUE_GATE, OUTPUT);
    digitalWrite(PINPYROMAIN_GATE, LOW);
    digitalWrite(PINPYRODROGUE_GATE, LOW);
}

void pyroFireMain(uint32_t ms) {
    digitalWrite(PINPYROMAIN_GATE, HIGH);
    delay(ms);
    digitalWrite(PINPYROMAIN_GATE, LOW);
}

void pyroFireDrogue(uint32_t ms) {
    digitalWrite(PINPYRODROGUE_GATE, HIGH);
    delay(ms);
    digitalWrite(PINPYRODROGUE_GATE, LOW);
}
