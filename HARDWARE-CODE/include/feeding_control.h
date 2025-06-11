#ifndef FEEDING_CONTROL_H
#define FEEDING_CONTROL_H

#include "config.h"
#include "globals.h"
#include "time_manager.h"    // Add this line
#include "network_manager.h" // Add this line for sendFeedingDataToPHP
#include "display_manager.h" // Add this line for buzzerBeepWithLED and setRGBColor

void handleFeeding();
void performAutoFeed();
bool canDispenseFood();
void recordFoodDispensing(String feedingType = "manual");
String getFeedingStatus();
void resetDailyCounters();

#endif