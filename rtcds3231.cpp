include "rtc_ds3231.h"

include "i2cmuxtca9548a.h"

include <RTClib.h>

static RTC_DS3231 rtc;

bool rtcInit() {
    tcaSelect(TCARTCDS3231);
    return rtc.begin();
}

bool rtcRead(RtcSample& out) {
    tcaSelect(TCARTCDS3231);
    DateTime now = rtc.now();
    out.unixTime = now.unixtime();
    out.valid = true;
    return true;
}
