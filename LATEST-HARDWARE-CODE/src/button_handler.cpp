#include "button_handler.h"

void handleButton(bool &currentButton, bool &lastButton, bool &buttonState, unsigned long &lastPress,
                  unsigned long currentMillis, void (*onPress)())
{
  if (currentButton != lastButton && currentMillis - lastPress > DEBOUNCE_DELAY)
  {
    if (currentButton)
    {
      buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_SHORT_BEEP, 0, RGB_WHITE);
      onPress();
      buttonState = true;
    }
    else
    {
      buttonState = false;
    }
    lastPress = currentMillis;
  }
  lastButton = currentButton;
}

void button1Action()
{
  feederSystem.refillMode = !feederSystem.refillMode;
  if (feederSystem.refillMode)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("REFILL MODE");
    buzzerBeepWithLED(BUZZER_PATTERN_WARNING, BUZZER_SHORT_BEEP, BUZZER_SHORT_PAUSE, RGB_YELLOW);
    setRGBColor(RGB_YELLOW);
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pet Feeder Ready");
    buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_LONG_BEEP, 0, RGB_GREEN);
    setRGBColor(String(sensors.foodLevel)); // Convert char array to String
  }
}

void button2Action()
{
  if (!feederSystem.refillMode && !feederSystem.dispensing && canDispenseFood())
  {
    myServo.write(180);
    feederSystem.dispensing = true;
    timing.dispenseStartTime = millis();
    recordFoodDispensing("Manual"); // Add the required String parameter
    buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_MEDIUM_BEEP, 0, RGB_PURPLE);
    setRGBColor(RGB_PURPLE);
  }
  else
  {
    buzzerBeepWithLED(BUZZER_PATTERN_DANGER, BUZZER_SHORT_BEEP, BUZZER_SHORT_PAUSE, RGB_RED);
  }
}

void handleButtons()
{
  unsigned long currentMillis = millis();
  bool currentButton1 = !digitalRead(BUTTON1_PIN);
  bool currentButton2 = !digitalRead(BUTTON2_PIN);

  handleButton(currentButton1, buttons.lastButton1, buttons.button1, timing.lastButton1Press, currentMillis, button1Action);
  handleButton(currentButton2, buttons.lastButton2, buttons.button2, timing.lastButton2Press, currentMillis, button2Action);
}