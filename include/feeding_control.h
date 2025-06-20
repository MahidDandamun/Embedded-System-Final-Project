#ifndef FEEDING_CONTROL_H
#define FEEDING_CONTROL_H

#include "config.h"
#include "globals.h"
#include "display_manager.h"
#include "network_manager.h"
#include "time_manager.h"

void handleFeeding();
void checkFeedingComplete();
void performAutoFeed();
bool canDispenseFood();
void recordFoodDispensing(String feedingType);
String getFeedingStatus(); // Keep this declaration here
void resetDailyCounters();
// Add these function declarations to your feeding_control.h
void handleRemoteFeeding();
bool canDispenseFoodRemote();
String getRemoteFeedingStatus();

#endif