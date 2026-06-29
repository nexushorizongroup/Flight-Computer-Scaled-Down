include "sd_logger.h"

include "telemetry_frame.h"

include "hardware_pins.h"

include "ring_buffer.h"

include <SdFat.h>

static SdFat sd;
static File  logFile;
static RingBuffer<FusionState,256> sdBacklog;

bool sdInit() {
    if (!sd.begin(PINSDCS, SDSCKMHZ(40))) return false;
    sd.card()->enableDma(true);
    char name[32];
    snprintf(name,sizeof(name),"/flight_%lu.bin",(unsigned long)millis());
    logFile = sd.open(name, FILE_WRITE);
    return logFile;
}

void sdLogBacklogPush(const FusionState& state) {
    sdBacklog.push(state);
}

void sdLogBacklogFlush() {
    if (!logFile) return;
    FusionState s;
    while (sdBacklog.popOldest(s)) {
        uint8_t buf[256];
        size_t len = buildFusionTelemetry(s, buf, sizeof(buf));
        if (len) logFile.write(buf,len);
    }
    logFile.flush();
}

void sdLogFusion(const FusionState& state) {
    if (!logFile) return;
    uint8_t buf[256];
    size_t len = buildFusionTelemetry(state, buf, sizeof(buf));
    if (len) {
        logFile.write(buf,len);
        logFile.flush();
    }
}
