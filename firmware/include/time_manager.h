#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "config.h"
#include "globals.h"

String formatDateTime(const RtcDateTime& dt);
String formatTime(const RtcDateTime& dt);
RtcDateTime getNextScheduledFeedTime(const RtcDateTime& currentTime);
bool shouldAutoFeed(const RtcDateTime& currentTime);
void setupTime();

#endif // TIME_MANAGER_H
