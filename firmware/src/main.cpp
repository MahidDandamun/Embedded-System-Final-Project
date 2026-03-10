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
#include "ble_manager.h"
#include "load_cell.h"

// ============================================================================
// Setup
// ============================================================================
void setup() {
    Serial.begin(115200);
    Serial.printf("\n=== Pet Feeder v%s ===\n", FIRMWARE_VERSION);

    unsigned long setupStart = millis();
    const unsigned long SETUP_TIMEOUT = 30000;

    // Core hardware init
    systemStart();

    // Load cell
    if (!scale.is_ready()) {
        setupLoadCell();
    }

    // Buttons
    initButtons();

    // BLE for mobile app
    initBLE();

    // Quick database connectivity test (skip if WiFi failed or timeout close)
    if (WiFi.status() == WL_CONNECTED &&
        millis() - setupStart < SETUP_TIMEOUT - 5000) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("DB Test...");
        bool dbOk = sendToDatabase();
        lcd.setCursor(0, 1);
        lcd.print(dbOk ? "Connected" : "Skipped");
        delay(500);
    }

    // Ready
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Ready!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.status() == WL_CONNECTED ? "Online Mode" : "Offline Mode");
    delay(1000);
    lcd.clear();

    feederSystem.initialized = true;
    Serial.printf("Setup done in %lu ms\n", millis() - setupStart);
}

// ============================================================================
// Main Loop
// ============================================================================
void loop() {
    if (!feederSystem.initialized) {
        delay(100);
        return;
    }

    unsigned long now = millis();

    // 1. Buttons first — manual override takes priority
    handleButtons();

    // 2. Feeding state machine (dispense timing, auto-feed check)
    if (!feederSystem.dispensing) {
        handleFeeding();
    }

    // 3. RTC & scheduled feeding
    if (feederSystem.rtcReady && !feederSystem.dispensing &&
        now - timing.lastRTCRead >= RTC_READ_INTERVAL) {
        RtcDateTime rtcNow = rtc.GetDateTime();

        String ts = formatTime(rtcNow);
        strncpy(timeData.currentTimeString, ts.c_str(),
                sizeof(timeData.currentTimeString) - 1);

        String nf = formatTime(timeData.nextScheduledFeed);
        strncpy(timeData.nextFeedTimeString, nf.c_str(),
                sizeof(timeData.nextFeedTimeString) - 1);

        if (shouldAutoFeed(rtcNow) && !feederSystem.dispensing) {
            Serial.println(">>> Auto-feed triggered");
            performAutoFeed();
        }
        timing.lastRTCRead = now;
    }

    // 4. Sensors
    handleSensors();

    if (now - timing.lastWeightRead >= WEIGHT_READ_INTERVAL) {
        updateBowlWeight();
        timing.lastWeightRead = now;
    }

    // 5. Check if dispense cycle finished
    checkFeedingComplete();

    // 6. BLE communication with mobile app
    handleBLE();

    // 7. Cloud sync
    handleBackendCommunication();

    // 8. LCD
    if (now - timing.lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
        updateLCD();
        timing.lastLCDUpdate = now;
    }

    // 9. Midnight reset
    resetDailyCounters();

    // 10. Periodic status dump
    if (now - timing.lastStatusPrint >= STATUS_PRINT_INTERVAL) {
        Serial.printf("\n=== STATUS ===\n");
        Serial.printf("WiFi: %s (%d dBm)\n",
                      WiFi.status() == WL_CONNECTED ? "OK" : "OFF", WiFi.RSSI());
        Serial.printf("MQTT: %s | BLE: %s\n",
                      feederSystem.mqttConnected ? "OK" : "OFF",
                      feederSystem.bleConnected  ? "OK" : "OFF");
        Serial.printf("Heap: %d B | Uptime: %lu s\n",
                      ESP.getFreeHeap(), now / 1000);
        Serial.println("==============\n");
        timing.lastStatusPrint = now;
    }
}
