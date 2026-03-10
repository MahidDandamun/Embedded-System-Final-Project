#include "button_handler.h"

static void handleButton(bool currentState, bool &lastState,
                          unsigned long &lastPress,
                          void (*onPress)()) {
    unsigned long now = millis();
    if (currentState != lastState && now - lastPress > DEBOUNCE_DELAY) {
        if (currentState) {
            buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_SHORT_BEEP, 0, RGB_WHITE);
            onPress();
        }
        lastPress = now;
    }
    lastState = currentState;
}

static void button1Action() {
    feederSystem.refillMode = !feederSystem.refillMode;
    if (feederSystem.refillMode) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("REFILL MODE");
        buzzerBeepWithLED(BUZZER_PATTERN_WARNING, BUZZER_SHORT_BEEP,
                          BUZZER_SHORT_PAUSE, RGB_YELLOW);
        setRGBColor(RGB_YELLOW);
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Pet Feeder Ready");
        buzzerBeepWithLED(BUZZER_PATTERN_SINGLE, BUZZER_LONG_BEEP, 0, RGB_GREEN);
        setRGBColor(sensors.foodLevel);
    }
}

static void button2Action() {
    if (!feederSystem.refillMode && !feederSystem.dispensing && canDispenseFood()) {
        performManualFeed();
    } else {
        buzzerBeepWithLED(BUZZER_PATTERN_DANGER, BUZZER_SHORT_BEEP,
                          BUZZER_SHORT_PAUSE, RGB_RED);
    }
}

void handleButtons() {
    bool btn1 = !digitalRead(BUTTON1_PIN);
    bool btn2 = !digitalRead(BUTTON2_PIN);

    handleButton(btn1, buttons.lastButton1, timing.lastButton1Press, button1Action);
    handleButton(btn2, buttons.lastButton2, timing.lastButton2Press, button2Action);
}

void initButtons() {
    buttons = ButtonState();
}
