#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "config.h"
#include "globals.h"

String formatDateTime(DateTime dt);
String formatTime(DateTime dt);
DateTime getNextScheduledFeedTime(DateTime currentTime);
bool shouldAutoFeed(DateTime currentTime);

#endif