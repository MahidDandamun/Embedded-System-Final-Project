#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "system_init.h"
#include "sensor_manager.h"
#include "feeding_control.h"
#include "button_handler.h"
#include "display_manager.h"
#include "time_manager.h"
#include "network_manager.h"

void setup()
{
  systemStart();
}

void loop()
{
  if (!feederSystem.initialized)
  {
    delay(100);
    return;
  }

  unsigned long currentMillis = millis();

  handleBackendCommunication();

  if (feederSystem.rtcReady && currentMillis - timing.lastRTCRead >= RTC_READ_INTERVAL)
  {
    DateTime now = rtc.now();
    timeData.currentTimeString = formatTime(now);
    timeData.nextFeedTimeString = formatTime(timeData.nextScheduledFeed);

    if (shouldAutoFeed(now))
      performAutoFeed();
    timing.lastRTCRead = currentMillis;
  }

  handleSensors();
  handleFeeding();
  handleButtons();

  if (currentMillis - timing.lastLCDUpdate >= LCD_UPDATE_INTERVAL)
  {
    updateLCD();
    timing.lastLCDUpdate = currentMillis;
  }

  resetDailyCounters();
}