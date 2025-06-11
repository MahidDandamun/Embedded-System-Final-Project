#include "feeding_control.h"

void handleFeeding()
{
  unsigned long currentMillis = millis();
  if (feederSystem.dispensing && currentMillis - timing.dispenseStartTime >= DISPENSE_TIME)
  {
    myServo.write(0);
    feederSystem.dispensing = false;
    buzzerBeepWithLED(BUZZER_PATTERN_WARNING, BUZZER_MEDIUM_BEEP, BUZZER_MEDIUM_PAUSE, RGB_PURPLE);
    setRGBColor(sensors.foodLevel);
  }
}

void performAutoFeed()
{
  if (canDispenseFood() && !feederSystem.dispensing)
  {
    myServo.write(180);
    feederSystem.dispensing = true;
    timing.dispenseStartTime = millis();
    recordFoodDispensing("auto");
    buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_MEDIUM_BEEP, 0, RGB_PURPLE);
    setRGBColor(RGB_PURPLE);
  }
}

bool canDispenseFood()
{
  unsigned long currentMillis = millis();

  // Check if daily limit reached
  if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
    return false;

  // Check if bowl is full
  if (feederSystem.bowlFull || sensors.bowlStatus == BOWL_STATUS_FULL)
    return false;

  // Check minimum interval between feedings
  if (currentMillis - timing.lastFeedingTime < MIN_FEEDING_INTERVAL)
    return false;

  // Check if food level is empty
  if (sensors.foodLevel == FOOD_LEVEL_EMPTY)
    return false;

  return true;
}

void recordFoodDispensing(String feedingType)
{
  float dispensedAmount = FOOD_PORTION_GRAMS;
  sensors.dailyFoodDispensed += dispensedAmount;
  sensors.totalFoodDispensed += dispensedAmount;
  timing.lastFeedingTime = millis();

  if (feederSystem.rtcReady)
  {
    timeData.lastAutoFeedTime = rtc.now();
    timeData.nextScheduledFeed = getNextScheduledFeedTime(rtc.now());
    sendFeedingDataToPHP(feedingType, dispensedAmount, formatDateTime(rtc.now()));
  }
}

String getFeedingStatus()
{
  unsigned long currentMillis = millis();

  if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
    return "Daily limit reached";

  if (feederSystem.bowlFull || sensors.bowlStatus == BOWL_STATUS_FULL)
    return "Bowl full";

  if (currentMillis - timing.lastFeedingTime < MIN_FEEDING_INTERVAL)
    return "Too soon";

  if (sensors.foodLevel == FOOD_LEVEL_EMPTY)
    return "No food";

  return "Ready to feed";
}

void resetDailyCounters()
{
  if (feederSystem.rtcReady)
  {
    DateTime now = rtc.now();
    static int lastDay = -1;

    if (lastDay != now.day())
    {
      sensors.dailyFoodDispensed = 0.0;
      lastDay = now.day();
    }
  }
}