#include "feeding_control.h"
#include "globals.h"
#include "display_manager.h"
void handleFeeding()
{
  // IMPORTANT: Only handle auto-feeding here
  // Manual feeding is handled directly in button_handler.cpp

  // Exit immediately if already dispensing to prevent multiple triggers
  if (feederSystem.dispensing)
  {
    return;
  }

  // Skip auto-feeding logic - only triggered by auto-feed or remote commands
  // Don't start dispensing automatically from this function
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

  // Check if food level is empty
  if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
  {
    Serial.println("Cannot dispense food - no food available");
    return;
  }

  // Use the EXACT same dispensing pattern as manual feeding
  Serial.println("=== STARTING REMOTE FEED SEQUENCE ===");

  // Set dispensing flag immediately to prevent multiple triggers
  feederSystem.dispensing = true;
  timing.dispenseStartTime = millis();

  // Display dispensing status on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Remote Dispensing");

  // Servo dispensing: 90° to 45° and back, 3 times with 1s delay (SAME AS MANUAL)
  for (int i = 0; i < 3; i++)
  {
    Serial.printf("Remote dispensing cycle %d/3\n", i + 1);

    // Update LCD with cycle number
    lcd.setCursor(0, 1);
    lcd.print("Cycle ");
    lcd.print(i + 1);
    lcd.print(" of 3     ");

    // Move servo to 45 degrees (left)
    myServo.write(45);

    // Buzzer sound when moving with LED feedback
    digitalWrite(BUZZER_PIN, HIGH);
    setRGBColor(RGB_BLUE); // Blue during remote dispensing
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    setRGBColor(RGB_OFF);

    delay(500); // Wait in position

    // Return servo to 90 degrees (center)
    myServo.write(90);

    // Buzzer sound when returning with LED feedback
    digitalWrite(BUZZER_PIN, HIGH);
    setRGBColor(RGB_BLUE);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    setRGBColor(RGB_OFF);

    // 1 second delay between cycles (except after last cycle)
    if (i < 2)
    {
      delay(1000);
    }
  }

  // Final confirmation (SAME AS MANUAL)
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Complete!");
  lcd.setCursor(0, 1);
  lcd.print("Food Dispensed");

  // Final buzzer sequence with LED
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    setRGBColor(RGB_GREEN); // Green for completion
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    setRGBColor(RGB_OFF);
    delay(150);
  }

  delay(500); // Reduced from 1000

  // Clear dispensing flag and update timing
  feederSystem.dispensing = false;
  timing.lastFeedingTime = millis();

  // Record the remote feeding
  recordFoodDispensing("Remote");

  // Restore food level LED
  updateFoodLevelLED();

  Serial.println("=== REMOTE FEEDING SEQUENCE COMPLETED ===");
  Serial.println("Ready for next dispensing");
}

// This function should be called in your main loop to check if dispensing is complete
void checkFeedingComplete()
{
  unsigned long currentMillis = millis();

  // This function is only for automatic timeout of dispensing
  // Manual feeding handles its own completion
  if (feederSystem.dispensing &&
      !buttons.manualFeedInProgress && // Don't interfere with manual feeding
      currentMillis - timing.dispenseStartTime >= DISPENSE_TIME)
  {
    // Stop dispensing
    myServo.write(90); // Return to rest position
    feederSystem.dispensing = false;

    // Provide completion feedback
    buzzerBeepWithLED(BUZZER_PATTERN_WARNING, BUZZER_MEDIUM_BEEP, BUZZER_MEDIUM_PAUSE, RGB_PURPLE);
    setRGBColor(sensors.foodLevel);

    Serial.println("Automatic feeding cycle completed (timed)");
  }
}

void performAutoFeed()
{
  // Only proceed if not already dispensing
  if (feederSystem.dispensing)
  {
    Serial.println("Cannot auto-feed - already dispensing");
    return;
  }

  Serial.println("=== AUTO FEED SEQUENCE STARTING ===");

  // Set dispensing flag immediately
  feederSystem.dispensing = true;
  timing.dispenseStartTime = millis();

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Auto Feeding");

  // Use the same dispensing pattern
  myServo.write(45); // Move to dispense position

  // Record and provide feedback
  recordFoodDispensing("Scheduled");
  buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_MEDIUM_BEEP, 0, RGB_BLUE);
  setRGBColor(RGB_BLUE);

  // Update last auto feed time
  if (feederSystem.rtcReady)
  {
    timeData.lastAutoFeedTime = rtc.GetDateTime();
  }

  Serial.println("Auto feeding started");
}

bool canDispenseFood()
{
  unsigned long currentMillis = millis();

  // Check minimum interval between feedings - reduced to 5 seconds for auto-feeding
  if (currentMillis - timing.lastFeedingTime < 5000) // 5 seconds
    return false;

  // Check if food level is empty - use strcmp for char array comparison
  if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
    return false;

  // REMOVED: Bowl full check and daily limit for manual feeding
  return true;
}

// New function specifically for manual feeding - always allow if not empty
bool canManualDispense()
{
  // Only check if food is available and not currently dispensing
  if (feederSystem.dispensing)
    return false;

  // Check if food level is empty
  if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
    return false;

  // Allow manual dispensing regardless of bowl status, timing, or daily limits
  return true;
}

// New function that can override timing for remote commands
bool canDispenseFoodRemote()
{
  // DISABLED: Daily limit check
  // if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
  //   return false;

  // Check if food level is empty - use strcmp for char array comparison
  if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
    return false;

  // Skip timing and bowl full checks for remote commands
  return true;
}

void recordFoodDispensing(String feedingType)
{
  // DISABLED: Daily food tracking
  // sensors.dailyFoodDispensed += FOOD_PORTION_GRAMS;
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

  // DISABLED: Daily limit check
  // if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
  //   return String("Daily limit reached");

  // DISABLED: Bowl full check
  // if (feederSystem.bowlFull || strcmp(sensors.bowlStatus, BOWL_STATUS_FULL) == 0)
  //   return String("Bowl full");

  if (currentMillis - timing.lastFeedingTime < 5000) // 5 seconds
    return String("Too soon");

  // Use strcmp for char array comparison
  if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
    return String("No food");

  return String("Ready to feed");
}

// New function for remote feeding status
String getRemoteFeedingStatus()
{
  // DISABLED: Daily limit check
  // if (sensors.dailyFoodDispensed >= MAX_DAILY_FOOD)
  //   return String("Daily limit reached");

  if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
    return String("No food");

  return String("Ready to feed");
}

void resetDailyCounters()
{
  if (feederSystem.rtcReady)
  {
    RtcDateTime now = rtc.GetDateTime();
    static int lastDay = -1;

    if (lastDay != now.Day())
    {
      // DISABLED: Daily counter reset
      // sensors.dailyFoodDispensed = 0.0;
      lastDay = now.Day();
    }
  }
}