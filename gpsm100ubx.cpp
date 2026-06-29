include "gpsm100ubx.h"

static HardwareSerial* gpsSerial = nullptr;
static const sizet RXBUF_SIZE = 512;
static uint8t rxBuf[RXBUF_SIZE];
static size_t  rxHead = 0;

static GpsSample gpsLast{};
static uint64_t lastMsgUs = 0;

static uint8_t ckA, ckB;
static uint8_t clsId, msgId;
static uint16_t payloadLen;
static uint16_t payloadCnt;
static uint8_t  payload[128];
static enum { SSYNC1, SSYNC2, SCLASS, SID, SLEN1, SLEN2, SPAYLOAD, SCKA, SCKB } ubxState = SSYNC1;

static void ubxReset() {
    ubxState = S_SYNC1;
    ckA = ckB = 0;
    payloadCnt = 0;
}

static void ubxUpdateChecksum(uint8_t b) {
    ckA += b;
    ckB += ckA;
}

static void handleNavPvt() {
    if (payloadLen < 92) return;
    GpsSample g{};
    g.valid = true;
    uint32t iTOW = (uint32t)&payload[0];
    int32t  lon  = (int32t)&payload[24];
    int32t  lat  = (int32t)&payload[28];
    int32t  height = (int32t)&payload[32];
    int32t  velN = (int32t)&payload[60];
    int32t  velE = (int32t)&payload[64];
    int32t  velD = (int32t)&payload[68];
    uint32t hAcc = (uint32t)&payload[40];
    uint32t vAcc = (uint32t)&payload[44];
    uint8_t  fixType = payload[20];
    uint8_t  numSV   = payload[23];

    g.towMs  = iTOW;
    g.latDeg = lat * 1e-7;
    g.lonDeg = lon * 1e-7;
    g.altM   = height * 1e-3;
    g.velN   = velN * 1e-3;
    g.velE   = velE * 1e-3;
    g.velD   = velD * 1e-3;
    g.hAcc   = hAcc * 1e-3;
    g.vAcc   = vAcc * 1e-3;
    g.fixType = fixType;
    g.sats    = numSV;

    gpsLast = g;
    lastMsgUs = micros();
}

static void parseByte(uint8_t b) {
    switch (ubxState) {
        case S_SYNC1:
            if (b == 0xB5) ubxState = S_SYNC2;
            break;
        case S_SYNC2:
            if (b == 0x62) ubxState = S_CLASS;
            else ubxReset();
            break;
        case S_CLASS:
            clsId = b;
            ckA = ckB = 0;
            ubxUpdateChecksum(b);
            ubxState = S_ID;
            break;
        case S_ID:
            msgId = b;
            ubxUpdateChecksum(b);
            ubxState = S_LEN1;
            break;
        case S_LEN1:
            payloadLen = b;
            ubxUpdateChecksum(b);
            ubxState = S_LEN2;
            break;
        case S_LEN2:
            payloadLen |= ((uint16_t)b << 8);
            ubxUpdateChecksum(b);
            if (payloadLen > sizeof(payload)) ubxReset();
            else { payloadCnt = 0; ubxState = S_PAYLOAD; }
            break;
        case S_PAYLOAD:
            payload[payloadCnt++] = b;
            ubxUpdateChecksum(b);
            if (payloadCnt >= payloadLen) ubxState = SCKA;
            break;
        case SCKA:
            if (b == ckA) ubxState = SCKB;
            else ubxReset();
            break;
        case SCKB:
            if (b == ckB) {
                if (clsId == 0x01 && msgId == 0x07) handleNavPvt();
            }
            ubxReset();
            break;
    }
}

void gpsInit(HardwareSerial& port) {
    gpsSerial = &port;
    gpsSerial->begin(115200, SERIAL8N1, PINGPSRX, PINGPS_TX, true);
    ubxReset();
}

void gpsUpdate() {
    if (!gpsSerial) return;
    size_t avail = gpsSerial->available();
    if (avail == 0) return;
    sizet space = RXBUF_SIZE - rxHead;
    size_t toRead = min(avail, space);
    size_t n = gpsSerial->readBytes(rxBuf + rxHead, toRead);
    size_t start = rxHead;
    rxHead = (rxHead + n) % RXBUFSIZE;
    for (size_t i = 0; i < n; i++) {
        sizet idx = (start + i) % RXBUF_SIZE;
        parseByte(rxBuf[idx]);
    }
    if (micros() - lastMsgUs > 500000) gpsLast.valid = false;
}

bool gpsGet(GpsSample& out) {
    out = gpsLast;
    return gpsLast.valid;
}
