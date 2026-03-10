#include "feeding_control.h"

void handleFeeding() {
    // Nothing to do if not currently dispensing
    // Auto-feed is triggered from main loop via shouldAutoFeed()
}

void performAutoFeed() {
    if (canDispenseFood() && !feederSystem.dispensing) {
        Serial.println("Auto dispensing...");
        myServo.write(180);
        feederSystem.dispensing = true;
        timing.dispenseStartTime = millis();
        recordFoodDispensing("auto");
        buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_MEDIUM_BEEP, 0, RGB_PURPLE);
        setRGBColor(RGB_PURPLE);
    }
}

void performManualFeed() {
    if (canDispenseFood() && !feederSystem.dispensing) {
        Serial.println("Manual dispensing...");
        myServo.write(180);
        feederSystem.dispensing = true;
        buttons.manualFeedInProgress = true;
        timing.dispenseStartTime = millis();
        recordFoodDispensing("manual");
        buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_MEDIUM_BEEP, 0, RGB_PURPLE);
        setRGBColor(RGB_PURPLE);
    }
}

bool canDispenseFood() {
    unsigned long now = millis();

    if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
        return false;

    if (feederSystem.bowlFull || strcmp(sensors.bowlStatus, BOWL_STATUS_FULL) == 0)
        return false;

    if (now - timing.lastFeedingTime < MIN_FEEDING_INTERVAL && timing.lastFeedingTime != 0)
        return false;

    if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
        return false;

    return true;
}

void checkFeedingComplete() {
    if (feederSystem.dispensing &&
        millis() - timing.dispenseStartTime >= DISPENSE_TIME) {
        myServo.write(0);
        feederSystem.dispensing = false;
        buttons.manualFeedInProgress = false;
        buzzerBeepWithLED(BUZZER_PATTERN_WARNING, BUZZER_MEDIUM_BEEP,
                          BUZZER_MEDIUM_PAUSE, RGB_PURPLE);
        setRGBColor(sensors.foodLevel);
        Serial.println("Dispensing complete");
    }
}

void recordFoodDispensing(const char* feedingType) {
    float dispensed = FOOD_PORTION_GRAMS;
    sensors.dailyFoodDispensed += dispensed;
    sensors.totalFoodDispensed += dispensed;
    timing.lastFeedingTime = millis();

    if (feederSystem.rtcReady) {
        RtcDateTime now = rtc.GetDateTime();
        timeData.lastAutoFeedTime = now;
        timeData.nextScheduledFeed = getNextScheduledFeedTime(now);
        String ts = formatDateTime(now);
        sendFeedingDataToAzure(feedingType, dispensed, ts.c_str());
    }
}

const char* getFeedingStatus() {
    if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
        return "Daily limit reached";
    if (feederSystem.bowlFull || strcmp(sensors.bowlStatus, BOWL_STATUS_FULL) == 0)
        return "Bowl full";
    if (millis() - timing.lastFeedingTime < MIN_FEEDING_INTERVAL && timing.lastFeedingTime != 0)
        return "Too soon";
    if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
        return "No food";
    return "Ready to feed";
}

void resetDailyCounters() {
    if (!feederSystem.rtcReady) return;

    RtcDateTime now = rtc.GetDateTime();
    static uint8_t lastDay = 0;

    if (lastDay != now.Day()) {
        sensors.dailyFoodDispensed = 0.0;
        lastDay = now.Day();
        Serial.println("Daily counters reset");
    }
}
