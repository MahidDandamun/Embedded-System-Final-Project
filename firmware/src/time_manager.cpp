#include "time_manager.h"
#include <time.h>

String formatDateTime(const RtcDateTime& dt) {
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             dt.Year(), dt.Month(), dt.Day(),
             dt.Hour(), dt.Minute(), dt.Second());
    return String(buf);
}

String formatTime(const RtcDateTime& dt) {
    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
             dt.Hour(), dt.Minute(), dt.Second());
    return String(buf);
}

RtcDateTime getNextScheduledFeedTime(const RtcDateTime& currentTime) {
    int currentMinutes = currentTime.Hour() * 60 + currentTime.Minute();

    for (int i = 0; i < numFeedingTimes; i++) {
        if (feedingTimes[i] > currentMinutes) {
            int h = feedingTimes[i] / 60;
            int m = feedingTimes[i] % 60;
            return RtcDateTime(currentTime.Year(), currentTime.Month(),
                               currentTime.Day(), h, m, 0);
        }
    }

    // Wrap to first feeding time tomorrow
    int h = feedingTimes[0] / 60;
    int m = feedingTimes[0] % 60;
    uint32_t tomorrow = currentTime.TotalSeconds() + 86400;
    RtcDateTime nextDay(tomorrow);
    return RtcDateTime(nextDay.Year(), nextDay.Month(), nextDay.Day(), h, m, 0);
}

bool shouldAutoFeed(const RtcDateTime& currentTime) {
    if (!feederSystem.autoFeedingEnabled || !feederSystem.scheduledFeedingMode)
        return false;

    int currentMinutes = currentTime.Hour() * 60 + currentTime.Minute();

    for (int i = 0; i < numFeedingTimes; i++) {
        if (abs(currentMinutes - feedingTimes[i]) <= 1) {
            // Avoid duplicate feeds — require 1 hour gap
            uint32_t elapsed = currentTime.TotalSeconds() -
                               timeData.lastAutoFeedTime.TotalSeconds();
            if (elapsed > 3600) {
                return true;
            }
        }
    }
    return false;
}

void setupTime() {
    // Sync NTP for TLS certificate validation
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print("Waiting for NTP sync");
    time_t now = time(nullptr);
    int retries = 0;
    while (now < 8 * 3600 * 2 && retries < 20) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
        retries++;
    }
    Serial.println(now > 8 * 3600 * 2 ? " OK" : " TIMEOUT");
}
