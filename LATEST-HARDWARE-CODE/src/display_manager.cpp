#include "display_manager.h"

void updateLCD()
{
  // First line: Time and connection status
  lcd.setCursor(0, 0);
  if (feederSystem.rtcReady && !feederSystem.dispensing)
  {
    lcd.print(formatTime(rtc.GetDateTime()));
    lcd.print(feederSystem.backendConnected ? " *" : " X");
    lcd.print(" ");

    // Display food level on first line
    if (strcmp(sensors.foodLevel, FOOD_LEVEL_FULL) == 0)
    {
      lcd.print("FULL ");
    }
    else if (strcmp(sensors.foodLevel, FOOD_LEVEL_HALF) == 0)
    {
      lcd.print("HALF ");
    }
    else
    {
      lcd.print("EMPTY");
    }
  }
  else if (feederSystem.dispensing)
  {
    lcd.print("Dispensing...   ");
  }
  else
  {
    lcd.print("Pet Feeder Ready");
  }

  // Second line: Next feeding time only (removed weight display)
  lcd.setCursor(0, 1);
  if (feederSystem.refillMode)
  {
    lcd.print("REFILLING...    ");
  }
  else if (feederSystem.dispensing)
  {
    lcd.print("Please wait...  ");
  }
  else
  {
    // Display next feeding time only
    if (feederSystem.rtcReady)
    {
      // Extract only HH:MM from next feed time
      char nextFeedTime[6];
      strncpy(nextFeedTime, timeData.nextFeedTimeString, 5);
      nextFeedTime[5] = '\0';

      lcd.print("Next Feed: ");
      lcd.print(nextFeedTime);
      lcd.print("   "); // Clear remaining chars
    }
    else
    {
      lcd.print("No Schedule     ");
    }
  }
}

void setRGBColor(String level)
{
  // Ensure RGB pins are properly configured
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  bool red = false, green = false, blue = false;

  if (level == RGB_GREEN || level == FOOD_LEVEL_FULL || strcmp(sensors.foodLevel, FOOD_LEVEL_FULL) == 0)
  {
    green = true; // Green for full food container
  }
  else if (level == RGB_RED || level == FOOD_LEVEL_EMPTY || strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
  {
    red = true; // Red for empty food container
  }
  else if (level == RGB_BLUE || level == FOOD_LEVEL_HALF || strcmp(sensors.foodLevel, FOOD_LEVEL_HALF) == 0)
  {
    blue = true; // Blue for half empty food container
  }
  else if (level == RGB_PURPLE || level == "VIOLET") // Violet/Purple for initialization
  {
    red = true;
    blue = true; // Red + Blue = Violet/Purple
  }
  else if (level == RGB_YELLOW)
  {
    red = true;
    green = true; // Red + Green = Yellow
  }
  else if (level == RGB_WHITE)
  {
    red = true;
    green = true;
    blue = true; // All colors = White
  }

  // Write to RGB pins with explicit HIGH/LOW
  digitalWrite(RED_PIN, red ? HIGH : LOW);
  digitalWrite(GREEN_PIN, green ? HIGH : LOW);
  digitalWrite(BLUE_PIN, blue ? HIGH : LOW);

  Serial.printf("RGB LED set: R=%d G=%d B=%d for level: %s\n", red, green, blue, level.c_str());
}

// Add initialization LED function
void setInitializationLED()
{
  setRGBColor(RGB_PURPLE); // Purple during initialization
}

// Add function to set food level LED
void updateFoodLevelLED()
{
  // Update LED based on current food level
  if (strcmp(sensors.foodLevel, FOOD_LEVEL_FULL) == 0)
  {
    setRGBColor(RGB_GREEN); // Green for full
  }
  else if (strcmp(sensors.foodLevel, FOOD_LEVEL_HALF) == 0)
  {
    setRGBColor(RGB_BLUE); // Blue for half
  }
  else if (strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
  {
    setRGBColor(RGB_RED); // Red for empty
  }
  else
  {
    setRGBColor(RGB_OFF); // Turn off if unknown
  }
}

void buzzerBeepWithLED(int beeps, int duration, int pause, String ledColor)
{
  for (int i = 0; i < beeps; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    setRGBColor(ledColor);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    setRGBColor(RGB_OFF);
    if (i < beeps - 1)
      delay(pause);
  }

  // Restore food level LED after buzzer sequence
  updateFoodLevelLED();
}