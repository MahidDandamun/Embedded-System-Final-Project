#include "feeding_control.h"

void handleFeeding()
{
  // Check if we can dispense food
  if (!canDispenseFood() || feederSystem.dispensing)
  {
    Serial.println("Cannot dispense food at this time");
    return;
  }

  // Start the feeding sequence
  Serial.println("Starting feeding sequence...");
  myServo.write(90); // Open the servo to dispense food
  feederSystem.dispensing = true;
  timing.dispenseStartTime = millis();

  // Record the food dispensing
  recordFoodDispensing("Manual/Remote");

  // Provide audio/visual feedback
  buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_MEDIUM_BEEP, 0, RGB_BLUE);
  setRGBColor(RGB_BLUE);

  Serial.println("Food dispensing started");
}

// New function for remote feeding that can override timing restrictions
void handleRemoteFeeding()
{
  // Only check critical restrictions for remote feeding
  if (feederSystem.dispensing)
  {
    Serial.println("Cannot dispense food - already dispensing");
    return;
  }

  // Check daily limit
  if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
  {
    Serial.println("Cannot dispense food - daily limit reached");
    return;
  }

  // Check if food level is empty
  if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
  {
    Serial.println("Cannot dispense food - no food available");
    return;
  }

  // Start the feeding sequence (bypass timing restrictions for remote)
  Serial.println("Starting REMOTE feeding sequence...");
  myServo.write(90); // Open the servo to dispense food
  feederSystem.dispensing = true;
  timing.dispenseStartTime = millis();

  // Record the food dispensing
  recordFoodDispensing("Remote");

  // Provide audio/visual feedback
  buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_MEDIUM_BEEP, 0, RGB_BLUE);
  setRGBColor(RGB_BLUE);

  Serial.println("Remote food dispensing started");
}

// This function should be called in your main loop to check if dispensing is complete
void checkFeedingComplete()
{
  unsigned long currentMillis = millis();
  if (feederSystem.dispensing && currentMillis - timing.dispenseStartTime >= DISPENSE_TIME)
  {
    // Stop dispensing
    myServo.write(0); // Close the servo
    feederSystem.dispensing = false;

    // Provide completion feedback
    buzzerBeepWithLED(BUZZER_PATTERN_WARNING, BUZZER_MEDIUM_BEEP, BUZZER_MEDIUM_PAUSE, RGB_PURPLE);
    setRGBColor(sensors.foodLevel);

    Serial.println("Food dispensing completed");
  }
}

void performAutoFeed()
{
  if (canDispenseFood() && !feederSystem.dispensing)
  {
    myServo.write(90);
    feederSystem.dispensing = true;
    timing.dispenseStartTime = millis();
    recordFoodDispensing("Scheduled");
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

// New function that can override timing for remote commands
bool canDispenseFoodRemote()
{
  // Check if daily limit reached
  if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
    return false;

  // Check if food level is empty - use strcmp for char array comparison
  if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
    return false;

  // Skip timing and bowl full checks for remote commands
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

// New function for remote feeding status
String getRemoteFeedingStatus()
{
  if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
    return String("Daily limit reached");

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