include "telemetry_frame.h"

include "util_crc.h"

include "util_cobs.h"

static sizet packFusionPayload(const FusionState& s, uint8t* buf, size_t maxLen) {
    uint8_t* p = buf;
    auto writeF = &{ memcpy(p,&v,sizeof(float)); p+=sizeof(float); };
    auto writeU32=&{ memcpy(p,&v,sizeof(uint32t)); p+=sizeof(uint32_t); };
    auto writeU8=&{ *p++=v; };

    writeU32((uint32_t)(s.tUs & 0xFFFFFFFF));
    writeF(s.alt); writeF(s.vel); writeF(s.accelBias);
    writeF(s.baroAlt); writeF(s.gpsAlt); writeF(s.gpsVelD);
    writeF(s.gpsQuality); writeU8(s.gpsValid);
    writeU8(s.ekfInconsistent); writeU8(s.gpsTimeout); writeU8(s.gpsAccuracyBad);
    writeU8(s.pyroMainArmed); writeU8(s.pyroMainFired);
    writeU8(s.pyroDrogueArmed); writeU8(s.pyroDrogueFired);
    writeF(s.power.vBatt); writeF(s.power.v5); writeF(s.power.v3v3);
    writeU8(s.vibHigh); writeU8(s.machLock); writeU8(s.padTiltBad);
    writeU8((uint8_t)s.phase);
    return p - buf;
}

size_t buildFusionTelemetry(const FusionState& state,
                            uint8t* outBuf, sizet outMax) {
    uint8_t payload[128];
    size_t payloadLen = packFusionPayload(state, payload, sizeof(payload));
    if (!payloadLen) return 0;
    TelemetryHeader hdr{};
    hdr.type = TLMTYPEFUSION_STATE;
    hdr.version = 1;
    hdr.length = payloadLen;
    hdr.crc = crc32_fast(payload, payloadLen);
    uint8_t frame[sizeof(TelemetryHeader)+sizeof(payload)];
    memcpy(frame,&hdr,sizeof(hdr));
    memcpy(frame+sizeof(hdr),payload,payloadLen);
    size_t frameLen = sizeof(hdr)+payloadLen;
    return cobsEncode(frame,frameLen,outBuf,outMax);
}
