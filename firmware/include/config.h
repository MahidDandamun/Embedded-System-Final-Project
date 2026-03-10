#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

// Load secrets from external file (never committed to git)
#include "secrets.h"

// ============================================================================
// WiFi Configuration
// ============================================================================
#define WIFI_SSID             ENV_WIFI_SSID
#define WIFI_PASSWORD         ENV_WIFI_PASSWORD
#define WIFI_RETRY_ATTEMPTS   20
#define WIFI_RETRY_DELAY_MS   500

// ============================================================================
// Device Configuration
// ============================================================================
#define DEVICE_NAME           "PetFeeder_001"
#define FIRMWARE_VERSION      "2.0.0"

// ============================================================================
// Pin Definitions (matches PCB v2 layout)
// ============================================================================
#define POWER_PIN             27
#define BUTTON1_PIN           15    // Refill mode toggle
#define BUTTON2_PIN           0     // Manual feed
#define ULTRASONIC_TRIG_PIN   32
#define ULTRASONIC_ECHO_PIN   33
#define PIR_PIN               25
#define HX711_DOUT_PIN        5
#define HX711_SCK_PIN         23
#define RED_PIN               13
#define GREEN_PIN             12
#define BLUE_PIN              14
#define BUZZER_PIN            26
#define SERVO_PIN             4

// DS1302 RTC Module Pins
#define RTC_IO                16
#define RTC_SCLK              17
#define RTC_CE                2

// ============================================================================
// LCD Configuration
// ============================================================================
#define LCD_ADDRESS           0x27
#define LCD_COLUMNS           16
#define LCD_ROWS              2

// ============================================================================
// Timing Intervals (milliseconds)
// ============================================================================
#define ULTRASONIC_READ_INTERVAL  200
#define WEIGHT_READ_INTERVAL      500
#define RTC_READ_INTERVAL         5000
#define LCD_UPDATE_INTERVAL       400
#define DATA_SYNC_INTERVAL        30000
#define DISPENSE_TIME             1000
#define MIN_FEEDING_INTERVAL      300000  // 5 minutes
#define PIR_TIMEOUT               30000   // 30 seconds
#define DEBOUNCE_DELAY            50
#define MQTT_RECONNECT_INTERVAL   10000
#define STATUS_PRINT_INTERVAL     30000

// ============================================================================
// Sensor Thresholds
// ============================================================================
#define FOOD_FULL_DISTANCE    9.0     // cm — container nearly full
#define FOOD_HALF_DISTANCE    13.5    // cm — roughly half
#define FOOD_EMPTY_DISTANCE   18.0    // cm — nearly empty
#define EMPTY_BOWL_THRESHOLD  10.0    // grams
#define FULL_BOWL_THRESHOLD   200.0   // grams
#define SCALE_READINGS        3

// ============================================================================
// Load Cell Calibration
// ============================================================================
#define CALIBRATION_FACTOR    49400   // Adjust during calibration

// ============================================================================
// Food Management
// ============================================================================
#define FOOD_PORTION_GRAMS    25.0
#define MAX_DAILY_FOOD        200.0   // grams per day

// Feeding Times (minutes from midnight)
#define FEEDING_TIME_1        480     // 08:00 AM
#define FEEDING_TIME_2        720     // 12:00 PM
#define FEEDING_TIME_3        1080    // 06:00 PM
#define FEEDING_TIME_4        1320    // 10:00 PM

// ============================================================================
// Azure IoT Hub / MQTT
// ============================================================================
#define MQTT_SERVER           ENV_MQTT_SERVER
#define MQTT_PORT             ENV_MQTT_PORT
#define DEVICE_ID             ENV_DEVICE_ID
#define SAS_TOKEN             ENV_SAS_TOKEN
#define MQTT_USERNAME         ENV_MQTT_USERNAME
#define DATABASE_ENDPOINT     ENV_DATABASE_ENDPOINT
#define MQTT_BUFFER_SIZE      1024

// ============================================================================
// BLE Configuration (for mobile app)
// ============================================================================
#define BLE_DEVICE_NAME       "PetFeeder"
#define BLE_SERVICE_UUID      "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHAR_CMD_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_CHAR_STATUS_UUID  "a3c87500-8ed3-4bdf-8a39-a01bebede295"

// ============================================================================
// String Constants
// ============================================================================
#define RGB_OFF               "OFF"
#define RGB_RED               "RED"
#define RGB_GREEN             "GREEN"
#define RGB_BLUE              "BLUE"
#define RGB_YELLOW            "YELLOW"
#define RGB_PURPLE            "PURPLE"
#define RGB_WHITE             "WHITE"

#define FOOD_LEVEL_FULL       "FULL"
#define FOOD_LEVEL_HALF       "HALF"
#define FOOD_LEVEL_EMPTY      "EMPTY"

#define BOWL_STATUS_EMPTY     "EMPTY"
#define BOWL_STATUS_PARTIAL   "PARTIAL"
#define BOWL_STATUS_FULL      "FULL"

// Buzzer Patterns
#define BUZZER_PATTERN_SINGLE   1
#define BUZZER_PATTERN_WARNING  3
#define BUZZER_PATTERN_DANGER   5
#define BUZZER_SHORT_BEEP       100
#define BUZZER_MEDIUM_BEEP      200
#define BUZZER_LONG_BEEP        500
#define BUZZER_SHORT_PAUSE      100
#define BUZZER_MEDIUM_PAUSE     200

// ============================================================================
// Utility Macros
// ============================================================================
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// ============================================================================
// Data Structures
// ============================================================================
struct SystemState {
    bool initialized          = false;
    bool powerOn              = false;
    bool rtcReady             = false;
    bool autoFeedingEnabled   = true;
    bool scheduledFeedingMode = true;
    bool refillMode           = false;
    bool dispensing           = false;
    bool animalDetected       = false;
    bool bowlFull             = false;
    bool weightBasedFeeding   = true;
    bool backendConnected     = false;
    bool mqttConnected        = false;
    bool bleConnected         = false;
    bool manualFeedRequested  = false;
};

struct ButtonState {
    bool button1              = false;
    bool button2              = false;
    bool lastButton1          = false;
    bool lastButton2          = false;
    bool manualFeedInProgress = false;
};

struct SensorData {
    float distance            = 0.0;
    float weight              = 0.0;
    bool  motionDetected      = false;
    char  foodLevel[10];
    char  bowlStatus[10];
    char  feedingStatus[25];
    float dailyFoodDispensed  = 0.0;
    float totalFoodDispensed  = 0.0;

    SensorData() {
        strcpy(foodLevel, FOOD_LEVEL_EMPTY);
        strcpy(bowlStatus, BOWL_STATUS_EMPTY);
        strcpy(feedingStatus, "Ready");
    }
};

struct Timing {
    unsigned long lastUltrasonicRead  = 0;
    unsigned long lastWeightRead      = 0;
    unsigned long lastRTCRead         = 0;
    unsigned long lastLCDUpdate       = 0;
    unsigned long lastDataSync        = 0;
    unsigned long lastFeedingTime     = 0;
    unsigned long lastMotionTime      = 0;
    unsigned long lastButton1Press    = 0;
    unsigned long lastButton2Press    = 0;
    unsigned long dispenseStartTime   = 0;
    unsigned long lastMQTTReconnect   = 0;
    unsigned long lastStatusPrint     = 0;
};

struct TimeData {
    char currentTimeString[20];
    char nextFeedTimeString[20];
    RtcDateTime lastAutoFeedTime;
    RtcDateTime nextScheduledFeed;

    TimeData() {
        memset(currentTimeString, 0, sizeof(currentTimeString));
        memset(nextFeedTimeString, 0, sizeof(nextFeedTimeString));
    }
};

struct IoTHubConfig {
    char mqttServer[100];
    int  mqttPort;
    char deviceId[20];
    char sasToken[300];
    char mqttUsername[100];
    char databaseEndpoint[100];
    char telemetryTopic[100];
    char methodTopic[50];

    IoTHubConfig() : mqttPort(MQTT_PORT) {
        strncpy(mqttServer, MQTT_SERVER, sizeof(mqttServer) - 1);
        strncpy(deviceId, DEVICE_ID, sizeof(deviceId) - 1);
        strncpy(sasToken, SAS_TOKEN, sizeof(sasToken) - 1);
        strncpy(mqttUsername, MQTT_USERNAME, sizeof(mqttUsername) - 1);
        strncpy(databaseEndpoint, DATABASE_ENDPOINT, sizeof(databaseEndpoint) - 1);
        snprintf(telemetryTopic, sizeof(telemetryTopic),
                 "devices/%s/messages/events/", DEVICE_ID);
        strncpy(methodTopic, "$iothub/methods/POST/#", sizeof(methodTopic) - 1);
    }
};

#endif // CONFIG_H
