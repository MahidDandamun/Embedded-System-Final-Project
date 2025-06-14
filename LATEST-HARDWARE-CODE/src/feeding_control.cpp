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
    recordFoodDispensing("Scheduled"); // Add the required String parameter
    buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_MEDIUM_BEEP, 0, RGB_BLUE);
    setRGBColor(RGB_BLUE);

    // Update last auto feed time
    if (feederSystem.rtcReady)
    {
      timeData.lastAutoFeedTime = rtc.now();
    }
  }
}

bool canDispenseFood()
{
  unsigned long currentMillis = millis();

  // Check if daily limit reached
  if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
    return false;

  // Check if bowl is full - use strcmp for char array comparison
  if (feederSystem.bowlFull || strcmp(sensors.bowlStatus, BOWL_STATUS_FULL) == 0)
    return false;

  // Check minimum interval between feedings
  if (currentMillis - timing.lastFeedingTime < MIN_FEEDING_INTERVAL)
    return false;

  // Check if food level is empty - use strcmp for char array comparison
  if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
    return false;

  return true;
}

void recordFoodDispensing(String feedingType)
{
  sensors.dailyFoodDispensed += FOOD_PORTION_GRAMS;
  sensors.totalFoodDispensed += FOOD_PORTION_GRAMS;
  timing.lastFeedingTime = millis();

  // Update feeding status
  String status = getFeedingStatus();
  strcpy(sensors.feedingStatus, status.c_str());

  // Log the feeding event
  if (feederSystem.rtcReady)
  {
    String timestamp = formatDateTime(rtc.now());
    sendFeedingDataToPHP(feedingType, FOOD_PORTION_GRAMS, timestamp);
  }

  Serial.println("Food dispensed: " + feedingType + " - " + String(FOOD_PORTION_GRAMS) + "g");
}

String getFeedingStatus()
{
  unsigned long currentMillis = millis();

  if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
    return String("Daily limit reached");

  // Use strcmp for char array comparison
  if (feederSystem.bowlFull || strcmp(sensors.bowlStatus, BOWL_STATUS_FULL) == 0)
    return String("Bowl full");

  if (currentMillis - timing.lastFeedingTime < MIN_FEEDING_INTERVAL)
    return String("Too soon");

  // Use strcmp for char array comparison
  if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
    return String("No food");

  return String("Ready to feed");
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