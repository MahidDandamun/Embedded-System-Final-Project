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

void testDataSending()
{
  Serial.println("\n=== MANUAL TEST: Sending sensor data ===");

  // Print current sensor values
  Serial.printf("Current sensor readings:\n");
  Serial.printf("- Distance: %.2f inches\n", sensors.distance);
  Serial.printf("- Weight: %.2f grams\n", sensors.weight);
  Serial.printf("- Motion detected: %s\n", sensors.motionDetected ? "Yes" : "No");
  Serial.printf("- Food level: %s\n", sensors.foodLevel);
  Serial.printf("- Bowl status: %s\n", sensors.bowlStatus);
  Serial.printf("- Feeding status: %s\n", sensors.feedingStatus);
  Serial.printf("- Daily food dispensed: %.2f grams\n", sensors.dailyFoodDispensed);
  Serial.printf("- WiFi connected: %s\n", WiFi.status() == WL_CONNECTED ? "Yes" : "No");
  Serial.printf("- WiFi RSSI: %d dBm\n", WiFi.RSSI());

  // Test Azure IoT Hub
  sendSensorDataToAzure();

  Serial.println("=== End of manual test ===\n");
}

void setup()
{
  systemStart();
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);

  // Add a delay then test (optional)
  delay(5000);
  testDataSending();
}

void loop()
{
  if (!feederSystem.initialized)
  {
    delay(100);
    return;
  }

  unsigned long currentMillis = millis();

  // Print connection status every 30 seconds
  static unsigned long lastStatusPrint = 0;
  if (currentMillis - lastStatusPrint >= 30000)
  {
    Serial.printf("\n=== STATUS UPDATE ===\n");
    Serial.printf("WiFi: %s (RSSI: %d dBm)\n",
                  WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected",
                  WiFi.RSSI());
    Serial.printf("MQTT: %s\n", feederSystem.mqttConnected ? "Connected" : "Disconnected");
    Serial.printf("Backend: %s\n", feederSystem.backendConnected ? "Connected" : "Disconnected");
    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println("====================\n");
    lastStatusPrint = currentMillis;
  }

  handleBackendCommunication();

  if (feederSystem.rtcReady && currentMillis - timing.lastRTCRead >= RTC_READ_INTERVAL)
  {
    DateTime now = rtc.now();

    String currentTimeStr = formatTime(now);
    strcpy(timeData.currentTimeString, currentTimeStr.c_str());

    String nextFeedStr = formatTime(timeData.nextScheduledFeed);
    strcpy(timeData.nextFeedTimeString, nextFeedStr.c_str());

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