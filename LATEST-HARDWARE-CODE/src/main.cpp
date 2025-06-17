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

void testDataSending();

void setup()
{
  systemStart();

  // Make sure button initialization happens early
  Serial.println("Initializing buttons...");
  initButtons();
  // Test database connection during initialization with strict timeout
  if (WiFi.status() == WL_CONNECTED)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Database Test");
    lcd.setCursor(0, 1);
    lcd.print("Sending data...");

    // Test database connection with strict timeout - don't block setup
    bool dbResult = false;
    unsigned long dbTestStart = millis();

    Serial.println("Starting database test with 8 second timeout...");

    // Create a separate task or use non-blocking approach
    while (millis() - dbTestStart < 8000) // 8 second max
    {
      dbResult = sendToDatabase();
      break; // Exit after first attempt regardless of result
    }

    if (millis() - dbTestStart >= 8000)
    {
      Serial.println("Database test timed out after 8 seconds, continuing...");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Database: SKIP");
      lcd.setCursor(0, 1);
      lcd.print("Timeout");
      delay(1000);
    }

    Serial.println("Database test completed, continuing with setup...");
  }
  else
  {
    Serial.println("WiFi not connected, skipping database test...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No WiFi");
    lcd.setCursor(0, 1);
    lcd.print("Skip DB Test");
    delay(1000);
  }

  // Show webserver starting
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Web Server");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(1000);

  lcd.setCursor(0, 1);
  lcd.print("Started!       ");
  delay(1000);

  // Final system ready message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  lcd.setCursor(0, 1);
  if (WiFi.status() == WL_CONNECTED && feederSystem.mqttConnected)
  {
    lcd.print("All Services OK");
  }
  else
  {
    lcd.print("Offline Mode");
  }
  delay(2000);

  // Clear LCD for normal operation
  lcd.clear();

  Serial.println("Setup complete - entering main loop");
}

void loop()
{
  if (!feederSystem.initialized)
  {
    delay(100);
    return;
  }

  unsigned long currentMillis = millis();

  // Handle buttons FIRST to catch manual dispense commands
  // This must be before handleFeeding() to ensure button presses override
  handleButtons();

  // Only check for feeding if not already dispensing
  if (!feederSystem.dispensing)
  {
    handleFeeding();
  }

  // Handle RTC and auto feeding ONLY if not dispensing
  if (feederSystem.rtcReady && !feederSystem.dispensing &&
      currentMillis - timing.lastRTCRead >= RTC_READ_INTERVAL)
  {
    RtcDateTime now = rtc.GetDateTime();

    String currentTimeStr = formatTime(now);
    strcpy(timeData.currentTimeString, currentTimeStr.c_str());

    String nextFeedStr = formatTime(timeData.nextScheduledFeed);
    strcpy(timeData.nextFeedTimeString, nextFeedStr.c_str());

    // Only check auto feeding if not manually dispensing
    if (shouldAutoFeed(now) && !feederSystem.dispensing)
    {
      Serial.println("=== AUTO FEED TRIGGERED ===");
      performAutoFeed();
    }
    timing.lastRTCRead = currentMillis;
  }

  // Handle sensors
  handleSensors();

  // Check if automatic feeding sequence is complete
  checkFeedingComplete();

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

  if (currentMillis - timing.lastLCDUpdate >= LCD_UPDATE_INTERVAL)
  {
    updateLCD();
    timing.lastLCDUpdate = currentMillis;
  }

  resetDailyCounters();
}

void testDataSending()
{
  Serial.println("\n=== MANUAL TEST: Sending sensor data ===");

  // Don't block if WiFi is not connected
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected, skipping data sending test");
    return;
  }

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

  // Test Azure IoT Hub with strict timeout - don't let it block
  unsigned long testStart = millis();
  Serial.println("Testing Azure IoT Hub with 5 second timeout...");

  // Set a flag to prevent blocking
  bool azureTestComplete = false;

  if (millis() - testStart < 5000) // 5 second max
  {
    sendSensorDataToAzure();
    azureTestComplete = true;
  }

  if (!azureTestComplete)
  {
    Serial.println("Azure test timed out after 5 seconds, continuing...");
  }

  Serial.println("=== End of manual test ===\n");
}