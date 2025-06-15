#include "time_manager.h"

String formatDateTime(DateTime dt)
{
  char buffer[20];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
          dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
  return String(buffer);
}

String formatTime(DateTime dt)
{
  char buffer[9];
  sprintf(buffer, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
  return String(buffer);
}

DateTime getPhilippineTime()
{
  if (!feederSystem.rtcReady)
    return DateTime(); // return default

  return rtc.now() + TimeSpan(0, 8, 0, 0); // Add 8 hours
}

DateTime getNextScheduledFeedTime(DateTime currentTime)
{
  int currentHour = currentTime.hour();
  int currentMinute = currentTime.minute();
  int currentTimeInMinutes = currentHour * 60 + currentMinute;

  for (int i = 0; i < numFeedingTimes; i++)
  {
    if (feedingTimes[i] > currentTimeInMinutes)
    {
      int nextHour = feedingTimes[i] / 60;
      int nextMinute = feedingTimes[i] % 60;
      return DateTime(currentTime.year(), currentTime.month(), currentTime.day(),
                      nextHour, nextMinute, 0);
    }
  }

  // If no feeding time today, return first feeding time of next day
  int nextHour = feedingTimes[0] / 60;
  int nextMinute = feedingTimes[0] % 60;
  DateTime nextDay = currentTime + TimeSpan(1, 0, 0, 0);
  return DateTime(nextDay.year(), nextDay.month(), nextDay.day(),
                  nextHour, nextMinute, 0);
}

bool shouldAutoFeed(DateTime currentTime)
{
  if (!feederSystem.autoFeedingEnabled || !feederSystem.scheduledFeedingMode)
    return false;

  int currentHour = currentTime.hour();
  int currentMinute = currentTime.minute();
  int currentTimeInMinutes = currentHour * 60 + currentMinute;

  for (int i = 0; i < numFeedingTimes; i++)
  {
    if (abs(currentTimeInMinutes - feedingTimes[i]) <= 1)
    {
      // Check if we haven't fed in the last hour to avoid duplicate feeds
      if (currentTime.unixtime() - timeData.lastAutoFeedTime.unixtime() > 3600)
      {
        return true;
      }
    }
  }

  return false;
}