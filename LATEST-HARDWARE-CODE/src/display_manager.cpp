#include "display_manager.h"

void updateLCD()
{
  if (WiFi.status() == WL_CONNECTED && !feederSystem.dispensing)
  {
    lcd.setCursor(0, 0);
    if (feederSystem.rtcReady)
    {
      lcd.print(formatTime(rtc.now()));
      lcd.print(feederSystem.backendConnected ? " *" : " X");
      lcd.print("   ");
    }
    else
    {
      lcd.print("Pet Feeder Ready");
    }

    lcd.setCursor(0, 1);
    if (feederSystem.refillMode)
    {
      lcd.print("REFILLING...    ");
    }
    else
    {
      // Build status line using char array operations
      char statusLine[50];
      strcpy(statusLine, "Food:");
      strcat(statusLine, sensors.foodLevel);

      if (feederSystem.rtcReady)
      {
        strcat(statusLine, " Next:");
        // Extract first 5 characters from nextFeedTimeString
        char timeSubstring[6];
        strncpy(timeSubstring, timeData.nextFeedTimeString, 5);
        timeSubstring[5] = '\0'; // Null terminate
        strcat(statusLine, timeSubstring);
      }
      strcat(statusLine, "      "); // Add padding spaces
      lcd.print(statusLine);
    }
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