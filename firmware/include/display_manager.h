#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "config.h"
#include "globals.h"
#include "time_manager.h"

void updateLCD();
void setRGBColor(const char* level);
void buzzerBeepWithLED(int beeps, int duration,
                       int pause = BUZZER_SHORT_PAUSE,
                       const char* ledColor = RGB_OFF);

#endif // DISPLAY_MANAGER_H
