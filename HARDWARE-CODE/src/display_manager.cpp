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
      String statusLine = "Food:" + sensors.foodLevel;
      if (feederSystem.rtcReady)
      {
        statusLine += " Next:" + timeData.nextFeedTimeString.substring(0, 5);
      }
      statusLine += "      ";
      lcd.print(statusLine);
    }
  }
}

void setRGBColor(String level)
{
  bool red = false, green = false, blue = false;

  if (level == RGB_GREEN || level == FOOD_LEVEL_FULL)
    green = true;
  else if (level == RGB_BLUE || level == FOOD_LEVEL_HALF)
    blue = true;
  else if (level == RGB_RED || level == FOOD_LEVEL_EMPTY)
    red = true;
  else if (level == RGB_YELLOW)
  {
    red = true;
    green = true;
  }
  else if (level == RGB_PURPLE)
  {
    red = true;
    blue = true;
  }
  else if (level == RGB_WHITE)
  {
    red = true;
    green = true;
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