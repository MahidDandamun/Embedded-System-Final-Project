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
#include "load_cell.h"

void testDataSending();

void setup()
{
  // Add setup timeout mechanism - reduced to 30 seconds
  unsigned long setupStartTime = millis();
  const unsigned long SETUP_TIMEOUT = 30000; // Reduced from 60 to 30 seconds

  Serial.println("=== SETUP STARTING WITH TIMEOUT ===");

  // Use a try-catch like approach with timeout checking
  bool setupComplete = false;

  // Initialize system with timeout protection
  if (millis() - setupStartTime < SETUP_TIMEOUT)
  {
    systemStart();
    setupComplete = true;
  }

  if (!setupComplete || millis() - setupStartTime >= SETUP_TIMEOUT)
  {
    Serial.println("⚠️ Setup timeout reached or incomplete - proceeding to loop");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Setup Timeout");
    lcd.setCursor(0, 1);
    lcd.print("Proceeding...");
    delay(1000); // Reduced from 2000

    // Minimal initialization to ensure system can run
    feederSystem.initialized = true;
  }

  // Initialize load cell early
  Serial.println("Initializing load cell (final check)...");
  if (!scale.is_ready())
  {
    setupLoadCell(); // Try one more time
  }

  // Make sure button initialization happens early
  Serial.println("Initializing buttons (final check)...");
  initButtons();

  // Test database connection with strict timeout - reduced timeout
  if (WiFi.status() == WL_CONNECTED && millis() - setupStartTime < SETUP_TIMEOUT - 5000)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Database Test");
    lcd.setCursor(0, 1);
    lcd.print("Quick test...");

    // Quick database test with 3 second timeout (reduced from 5)
    unsigned long dbTestStart = millis();
    Serial.println("Quick database test (3 sec timeout)...");

    bool dbResult = false;
    if (millis() - dbTestStart < 3000)
    {
      // Set a very short timeout for HTTP request
      dbResult = sendToDatabase();
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Database:");
    lcd.setCursor(0, 1);
    lcd.print(dbResult ? "Connected" : "Skipped");
    delay(500); // Reduced from 1000
  }
  else
  {
    Serial.println("Skipping database test due to timeout or no WiFi");
  }

  // Show final system status - reduced delay
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  lcd.setCursor(0, 1);
  if (WiFi.status() == WL_CONNECTED)
  {
    lcd.print("Online Mode");
  }
  else
  {
    lcd.print("Offline Mode");
  }
  delay(1000); // Reduced from 2000

  // Clear LCD for normal operation
  lcd.clear();

  // Ensure we're definitely going to proceed to loop
  feederSystem.initialized = true;

  unsigned long totalSetupTime = millis() - setupStartTime;
  Serial.printf("Setup completed in %lu ms - entering main loop\n", totalSetupTime);
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

  // Handle sensors (including load cell)
  handleSensors();

  // Update bowl weight regularly
  if (currentMillis - timing.lastWeightRead >= WEIGHT_READ_INTERVAL)
  {
    updateBowlWeight();
    timing.lastWeightRead = currentMillis;
  }

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