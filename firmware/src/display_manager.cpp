#include "display_manager.h"

void updateLCD() {
    if (feederSystem.dispensing) return;

    lcd.setCursor(0, 0);
    if (feederSystem.rtcReady) {
        lcd.print(timeData.currentTimeString);
        lcd.print(feederSystem.mqttConnected ? " *" : " X");
        lcd.print("   ");
    } else {
        lcd.print("Pet Feeder Ready");
    }

    lcd.setCursor(0, 1);
    if (feederSystem.refillMode) {
        lcd.print("REFILLING...    ");
    } else {
        char line[17];
        snprintf(line, sizeof(line), "F:%-4s N:%.5s",
                 sensors.foodLevel,
                 feederSystem.rtcReady ? timeData.nextFeedTimeString : "--:--");
        lcd.print(line);
    }
}

void setRGBColor(const char* level) {
    bool r = false, g = false, b = false;

    if (strcmp(level, RGB_GREEN) == 0 || strcmp(level, FOOD_LEVEL_FULL) == 0)
        g = true;
    else if (strcmp(level, RGB_BLUE) == 0 || strcmp(level, FOOD_LEVEL_HALF) == 0)
        b = true;
    else if (strcmp(level, RGB_RED) == 0 || strcmp(level, FOOD_LEVEL_EMPTY) == 0)
        r = true;
    else if (strcmp(level, RGB_YELLOW) == 0) { r = true; g = true; }
    else if (strcmp(level, RGB_PURPLE) == 0) { r = true; b = true; }
    else if (strcmp(level, RGB_WHITE) == 0)  { r = true; g = true; b = true; }

    digitalWrite(RED_PIN,   r);
    digitalWrite(GREEN_PIN, g);
    digitalWrite(BLUE_PIN,  b);
}

void buzzerBeepWithLED(int beeps, int duration, int pause, const char* ledColor) {
    for (int i = 0; i < beeps; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        setRGBColor(ledColor);
        delay(duration);
        digitalWrite(BUZZER_PIN, LOW);
        setRGBColor(RGB_OFF);
        if (i < beeps - 1) delay(pause);
    }
}
