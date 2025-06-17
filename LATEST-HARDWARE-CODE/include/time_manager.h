#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "config.h"
#include "globals.h"

String formatTime(const RtcDateTime &dt);
String formatDateTime(const RtcDateTime &dt);
RtcDateTime getPhilippineTime();
RtcDateTime getNextScheduledFeedTime(const RtcDateTime &currentTime);
bool shouldAutoFeed(const RtcDateTime &currentTime);

#endif