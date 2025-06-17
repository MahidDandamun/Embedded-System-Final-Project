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

  // Second line: Bowl weight and next feeding time
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
    // Display bowl weight and next feeding time
    char statusLine[17]; // 16 chars + null terminator

    // Format bowl weight with 1 decimal place
    char weightStr[8];
    dtostrf(sensors.weight, 4, 1, weightStr);

    if (feederSystem.rtcReady)
    {
      // Extract only HH:MM from next feed time
      char nextFeedTime[6];
      strncpy(nextFeedTime, timeData.nextFeedTimeString, 5);
      nextFeedTime[5] = '\0';

      snprintf(statusLine, sizeof(statusLine), "%sg Next:%s",
               weightStr, nextFeedTime);
    }
    else
    {
      snprintf(statusLine, sizeof(statusLine), "Bowl:%sg      ", weightStr);
    }

    lcd.print(statusLine);
  }
}

void setRGBColor(String level)
{
  bool red = false, green = false, blue = false;

  if (level == RGB_GREEN || level == FOOD_LEVEL_FULL || strcmp(sensors.foodLevel, FOOD_LEVEL_FULL) == 0)
  {
    green = true;
  }
  else if (level == RGB_RED || level == FOOD_LEVEL_EMPTY || strcmp(sensors.foodLevel, FOOD_LEVEL_EMPTY) == 0)
  {
    red = true;
  }
  else if (level == RGB_BLUE || level == FOOD_LEVEL_HALF || strcmp(sensors.foodLevel, FOOD_LEVEL_HALF) == 0)
  {
    blue = true;
  }

  digitalWrite(RED_PIN, red);
  digitalWrite(GREEN_PIN, green);
  digitalWrite(BLUE_PIN, blue);
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
}