#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "config.h"
#include "globals.h"
#include "display_manager.h" // Add this line for buzzerBeepWithLED and setRGBColor
#include "feeding_control.h" // Add this line for canDispenseFood and recordFoodDispensing

void handleButtons();
void handleButton(bool &currentButton, bool &lastButton, bool &buttonState,
                  unsigned long &lastPress, unsigned long currentMillis, void (*onPress)());
void button1Action();
void button2Action();

#endif