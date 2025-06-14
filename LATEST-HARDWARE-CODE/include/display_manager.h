#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "config.h"
#include "globals.h"
#include "time_manager.h" // Add this line

void updateLCD();
void setRGBColor(String level);
void buzzerBeepWithLED(int beeps, int duration, int pause = BUZZER_SHORT_PAUSE, String ledColor = RGB_OFF);

#endif