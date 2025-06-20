#ifndef CONFIG_H
#define CONFIG_H

// Include necessary headers for types used in structs
#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "secrets.h"
// WiFi Configuration
#define WIFI_SSID ENV_WIFI_SSID
#define WIFI_PASSWORD ENV_WIFI_PASSWORD
#define WIFI_RETRY_ATTEMPTS 20

// Device Configuration
#define DEVICE_NAME "PetFeeder_001"

// Pin Definitions
#define POWER_PIN 27
#define BUTTON1_PIN 15
#define BUTTON2_PIN 0
#define ULTRASONIC_ECHO_PIN 33
#define ULTRASONIC_TRIG_PIN 32
#define PIR_PIN 25
#define HX711_DOUT_PIN 5
#define HX711_SCK_PIN 23
#define RED_PIN 13
#define GREEN_PIN 12
#define BLUE_PIN 14
#define BUZZER_PIN 26
#define SERVO_PIN 4

// DS1302 RTC Module Pins
#define RTC_IO 16
#define RTC_SCLK 17
#define RTC_CE 2

// LCD Configuration
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Timing Intervals (in milliseconds)
#define ULTRASONIC_READ_INTERVAL 200
#define WEIGHT_READ_INTERVAL 500
#define RTC_READ_INTERVAL 5000
#define LCD_UPDATE_INTERVAL 400
#define DATA_SYNC_INTERVAL 30000
#define DISPENSE_TIME 1000
#define MIN_FEEDING_INTERVAL 300000 // 5 minutes
#define PIR_TIMEOUT 30000           // 30 seconds
#define DEBOUNCE_DELAY 50

// Sensor Thresholds
#define FOOD_FULL_DISTANCE 9.0    // cm
#define FOOD_HALF_DISTANCE 13.5   // cm
#define FOOD_EMPTY_DISTANCE 18.0  // cm
#define EMPTY_BOWL_THRESHOLD 10.0 // grams
#define FULL_BOWL_THRESHOLD 200.0 // grams
#define SCALE_READINGS 3          // Increased for better stability

// Food Management
#define FOOD_PORTION_GRAMS 25.0
#define MAX_DAILY_FOOD 200.0 // grams per day

// Feeding Times (in minutes from midnight)
#define FEEDING_TIME_1 480  // 8:00 AM
#define FEEDING_TIME_2 720  // 12:00 PM
#define FEEDING_TIME_3 1080 // 6:00 PM
#define FEEDING_TIME_4 1320 // 10:00 PM

// RGB Color Definitions
#define RGB_OFF "OFF"
#define RGB_RED "RED"
#define RGB_GREEN "GREEN"
#define RGB_BLUE "BLUE"
#define RGB_YELLOW "YELLOW"
#define RGB_PURPLE "PURPLE"
#define RGB_WHITE "WHITE"

// Food Level Definitions
#define FOOD_LEVEL_FULL "FULL"
#define FOOD_LEVEL_HALF "HALF"
#define FOOD_LEVEL_EMPTY "EMPTY"

// Bowl Status Definitions
#define BOWL_STATUS_EMPTY "EMPTY"
#define BOWL_STATUS_PARTIAL "PARTIAL"
#define BOWL_STATUS_FULL "FULL"

// Buzzer Patterns
#define BUZZER_PATTERN_SINGLE 1
#define BUZZER_PATTERN_WARNING 3
#define BUZZER_PATTERN_DANGER 5
#define BUZZER_SHORT_BEEP 100
#define BUZZER_MEDIUM_BEEP 200
#define BUZZER_LONG_BEEP 500
#define BUZZER_SHORT_PAUSE 100
#define BUZZER_MEDIUM_PAUSE 200

// Azure IoT Hub Configuration
#define MQTT_SERVER ENV_MQTT_SERVER
#define MQTT_PORT ENV_MQTT_PORT
#define DEVICE_ID ENV_DEVICE_ID
#define SAS_TOKEN ENV_SAS_TOKEN
#define MQTT_USERNAME ENV_MQTT_USERNAME
#define DATABASE_ENDPOINT ENV_DATABASE_ENDPOINT

// MQTT Buffer Size
#define MQTT_BUFFER_SIZE 1024

// Load Cell Configuration
#define CALIBRATION_FACTOR 49400 // Match the calibration factor from working code
#define SCALE_READINGS 3         // Increased for better stability

// Utility Macros
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// Forward declarations
class String;

// Structure Definitions
struct SystemState
{
  bool initialized = false;
  bool powerOn = false;
  bool rtcReady = false;
  bool autoFeedingEnabled = true;
  bool scheduledFeedingMode = true;
  bool refillMode = false;
  bool dispensing = false;
  bool animalDetected = false;
  bool bowlFull = false;
  bool weightBasedFeeding = true;
  bool backendConnected = false;
  bool mqttConnected = false;
};

struct ButtonState
{
  bool button1 = false;
  bool button2 = false;
  bool lastButton1 = false;
  bool lastButton2 = false;
  bool manualFeedInProgress = false; // Add this field for manual feeding tracking
};

struct SensorData
{
  float distance;
  float weight;
  bool motionDetected;
  char foodLevel[10];     // Use char array instead of String
  char bowlStatus[10];    // Use char array instead of String
  char feedingStatus[25]; // Use char array instead of String
  float dailyFoodDispensed;
  float totalFoodDispensed;

  // Constructor to initialize the values
  SensorData() : distance(0.0),
                 weight(0.0),
                 motionDetected(false),
                 dailyFoodDispensed(0.0),
                 totalFoodDispensed(0.0)
  {
    strcpy(foodLevel, FOOD_LEVEL_EMPTY);
    strcpy(bowlStatus, BOWL_STATUS_EMPTY);
    strcpy(feedingStatus, "Ready");
  }
};

struct Timing
{
  unsigned long lastUltrasonicRead = 0;
  unsigned long lastWeightRead = 0;
  unsigned long lastRTCRead = 0;
  unsigned long lastLCDUpdate = 0;
  unsigned long lastDataSync = 0;
  unsigned long lastFeedingTime = 0;
  unsigned long lastMotionTime = 0;
  unsigned long lastButton1Press = 0;
  unsigned long lastButton2Press = 0;
  unsigned long dispenseStartTime = 0;
  unsigned long lastMQTTReconnect = 0;
};

struct TimeData
{
  char currentTimeString[20];  // Use char array instead of String
  char nextFeedTimeString[20]; // Use char array instead of String
  RtcDateTime lastAutoFeedTime;
  RtcDateTime nextScheduledFeed;

  // Constructor
  TimeData()
  {
    strcpy(currentTimeString, "");
    strcpy(nextFeedTimeString, "");
  }
};

struct IoTHubConfig
{
  char mqttServer[100];
  int mqttPort;
  char deviceId[20];
  char sasToken[300];
  char mqttUsername[100];
  char databaseEndpoint[100];
  char telemetryTopic[100];
  char methodTopic[50];

  // Constructor
  IoTHubConfig() : mqttPort(MQTT_PORT)
  {
    strcpy(mqttServer, MQTT_SERVER);
    strcpy(deviceId, DEVICE_ID);
    strcpy(sasToken, SAS_TOKEN);
    strcpy(mqttUsername, MQTT_USERNAME);
    strcpy(databaseEndpoint, DATABASE_ENDPOINT);
    snprintf(telemetryTopic, sizeof(telemetryTopic), "devices/%s/messages/events/", DEVICE_ID);
    strcpy(methodTopic, "$iothub/methods/POST/#");
  }
};

#endif