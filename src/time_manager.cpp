#include "time_manager.h"

String formatTime(const RtcDateTime &dt)
{
  char timeStr[20];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d",
           dt.Hour(), dt.Minute(), dt.Second());
  return String(timeStr);
}

String formatDateTime(const RtcDateTime &dt)
{
  char dateTimeStr[30];
  snprintf(dateTimeStr, sizeof(dateTimeStr), "%04d-%02d-%02d %02d:%02d:%02d",
           dt.Year(), dt.Month(), dt.Day(),
           dt.Hour(), dt.Minute(), dt.Second());
  return String(dateTimeStr);
}

RtcDateTime getPhilippineTime()
{
  RtcDateTime now = rtc.GetDateTime();

  // Add 8 hours for Philippine Time (UTC+8) plus 36 minutes adjustment (24+12 more)
  int hoursToAdd = 8;
  int minutesToAdd = 36; // Increased from 24 to 36 minutes

  int newMinute = now.Minute() + minutesToAdd;
  int newHour = now.Hour() + hoursToAdd;
  int newDay = now.Day();
  int newMonth = now.Month();
  int newYear = now.Year();

  // Handle minute overflow
  while (newMinute >= 60)
  {
    newMinute -= 60;
    newHour += 1;
  }

  // Handle hour overflow
  while (newHour >= 24)
  {
    newHour -= 24;
    newDay += 1;
  }

  // Handle day overflow (simplified - assumes 31 days per month)
  if (newDay > 31)
  {
    newDay = 1;
    newMonth += 1;

    // Handle month overflow
    if (newMonth > 12)
    {
      newMonth = 1;
      newYear += 1;
    }
  }

  return RtcDateTime(newYear, newMonth, newDay, newHour, newMinute, now.Second());
}

RtcDateTime getNextScheduledFeedTime(const RtcDateTime &currentTime)
{
  int currentMinutes = currentTime.Hour() * 60 + currentTime.Minute();

  // Check each feeding time
  for (int i = 0; i < numFeedingTimes; i++)
  {
    if (feedingTimes[i] > currentMinutes)
    {
      // Found next feeding time today
      int hours = feedingTimes[i] / 60;
      int minutes = feedingTimes[i] % 60;

      return RtcDateTime(currentTime.Year(), currentTime.Month(), currentTime.Day(),
                         hours, minutes, 0);
    }
  }

  // No more feeding times today, return first feeding time tomorrow
  int newDay = currentTime.Day() + 1;
  int newMonth = currentTime.Month();
  int newYear = currentTime.Year();

  // Simple day overflow handling (doesn't handle month boundaries properly)
  if (newDay > 31)
  {
    newDay = 1;
    newMonth += 1;
  }

  int hours = feedingTimes[0] / 60;
  int minutes = feedingTimes[0] % 60;

  return RtcDateTime(newYear, newMonth, newDay, hours, minutes, 0);
}

bool shouldAutoFeed(const RtcDateTime &currentTime)
{
  if (!feederSystem.rtcReady || !feederSystem.autoFeedingEnabled)
    return false;

  int currentMinutes = currentTime.Hour() * 60 + currentTime.Minute();

  // Check if it's time for any scheduled feeding
  for (int i = 0; i < numFeedingTimes; i++)
  {
    if (abs(currentMinutes - feedingTimes[i]) <= 1) // Within 1 minute
    {
      // Check if we haven't fed in the last hour
      RtcDateTime lastFeed = timeData.lastAutoFeedTime;

      // Simple time comparison - check if at least 1 hour has passed
      int lastFeedMinutes = lastFeed.Hour() * 60 + lastFeed.Minute();
      int timeDiff = currentMinutes - lastFeedMinutes;

      // Handle day boundary
      if (timeDiff < 0)
      {
        timeDiff += 24 * 60; // Add 24 hours worth of minutes
      }

      if (timeDiff >= 60) // At least 1 hour has passed
      {
        return true;
      }
    }
  }

  return false;
}