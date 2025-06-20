#ifndef LOAD_CELL_H
#define LOAD_CELL_H

#include <Arduino.h>
#include "HX711.h"
#include "globals.h"
#include "config.h"

// Function declarations
void setupLoadCell();
float getWeight();
void updateBowlWeight();
void handleButtonPress();
void dispenseFood();
void playBuzzer(int beepCount = 1, int beepDuration = BUZZER_SHORT_BEEP, int pauseDuration = BUZZER_SHORT_PAUSE);
void displayMessage(String line1, String line2 = "");
void displayWeight(float weight);

extern HX711 scale;

#endif // LOAD_CELL_H
