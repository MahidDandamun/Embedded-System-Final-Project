#ifndef FEEDING_CONTROL_H
#define FEEDING_CONTROL_H

#include "config.h"
#include "globals.h"
#include "time_manager.h"
#include "network_manager.h"
#include "display_manager.h"

void handleFeeding();
void performAutoFeed();
void performManualFeed();
bool canDispenseFood();
void checkFeedingComplete();
void recordFoodDispensing(const char* feedingType = "manual");
const char* getFeedingStatus();
void resetDailyCounters();

#endif // FEEDING_CONTROL_H
