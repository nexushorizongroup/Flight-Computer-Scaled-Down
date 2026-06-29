pragma once

include <Arduino.h>

static const int PINI2CSDA = 8;
static const int PINI2CSCL = 9;

static const uint8t TCA9548AADDR = 0x70;

enum TcaBus : uint8_t {
    TCAIMUICM20948 = 0,
    TCABAROMS5611  = 1,
    TCA_GY91         = 2,
    TCARTCDS3231   = 3,
    TCA_INA3221      = 4,
    TCA_RESERVED5    = 5,
    TCA_RESERVED6    = 6,
    TCA_RESERVED7    = 7
};

static const int PINGPSTX = 17;
static const int PINGPSRX = 18;
static const int PINGPSPPS = 4;

static const int PINSPIMOSI = 11;
static const int PINSPIMISO = 13;
static const int PINSPISCK  = 12;

static const int PINSDCS    = 5;
static const int PINLORACS  = 6;
static const int PINLORARST = 7;
static const int PINLORADIO = 10;

static const int PINVIBADC  = 3;

static const int PINPYROMAIN_GATE   = 14;
static const int PINPYRODROGUE_GATE = 15;
static const int PINPYROMAIN_SENSE   = 1;
static const int PINPYRODROGUE_SENSE = 2;
