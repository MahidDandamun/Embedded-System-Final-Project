#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <RTClib.h>

// WiFi Configuration
#define WIFI_SSID "Your_WiFi_SSID"         // Replace with your WiFi name
#define WIFI_PASSWORD "Your_WiFi_Password" // Replace with your WiFi password
#define WIFI_RETRY_ATTEMPTS 20

// Server Configuration
#define PHP_SERVER_URL "http://192.168.1.100" // Replace with your computer's IP
#define DEVICE_NAME "PetFeeder Group 2"

// --- Pin Definitions ---
#define POWER_PIN 2
#define BUTTON1_PIN 15
#define BUTTON2_PIN 4
#define ULTRASONIC_TRIG_PIN 5
#define ULTRASONIC_ECHO_PIN 18
#define PIR_PIN 19
#define HX711_DOUT_PIN 16
#define HX711_SCK_PIN 17
#define RED_PIN 25
#define GREEN_PIN 26
#define BLUE_PIN 27
#define BUZZER_PIN 14
#define SERVO_PIN 13

// --- LCD Configuration ---
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// --- Scale Configuration ---
#define CALIBRATION_FACTOR -7050.0
#define WEIGHT_CHANGE_THRESHOLD 5.0

// --- Feeding Configuration ---
#define NORMAL_PORTION_SIZE 50.0
#define MAX_DAILY_FOOD 200.0
#define MIN_FEEDING_INTERVAL 300000
#define DISPENSE_TIME 3000

// --- Thresholds ---
#define EMPTY_BOWL_THRESHOLD 10.0
#define FULL_BOWL_THRESHOLD 100.0
#define ULTRASONIC_OFFSET 2.0
#define FOOD_FULL_DISTANCE 5.0
#define FOOD_HALF_DISTANCE 10.0

// --- Timing Intervals (milliseconds) ---
#define RTC_READ_INTERVAL 1000
#define WEIGHT_READ_INTERVAL 1000
#define PIR_READ_INTERVAL 500
#define DISTANCE_READ_INTERVAL 2000
#define DATA_SYNC_INTERVAL 30000
#define LCD_UPDATE_INTERVAL 1000
#define LOW_FOOD_BEEP_INTERVAL 60000
#define EMPTY_WARNING_INTERVAL 30000
#define DAILY_RESET_INTERVAL 86400000
#define AUTO_FEED_INTERVAL 14400

// --- Feeding Times (24-hour format) ---
#define FEEDING_TIME_1 8
#define FEEDING_TIME_2 12
#define FEEDING_TIME_3 18
#define FEEDING_TIME_4 22

// --- Buzzer Patterns ---
#define BUZZER_PATTERN_SINGLE 1
#define BUZZER_PATTERN_WARNING 2
#define BUZZER_PATTERN_DANGER 3
#define BUZZER_PATTERN_LOW_FOOD 5

// --- Buzzer Durations ---
#define BUZZER_SHORT_BEEP 100
#define BUZZER_MEDIUM_BEEP 300
#define BUZZER_LONG_BEEP 500
#define BUZZER_SHORT_PAUSE 100
#define BUZZER_MEDIUM_PAUSE 300
#define BUZZER_LONG_PAUSE 500

// --- RGB Colors ---
#define RGB_OFF "OFF"
#define RGB_RED "RED"
#define RGB_GREEN "GREEN"
#define RGB_BLUE "BLUE"
#define RGB_YELLOW "YELLOW"
#define RGB_PURPLE "PURPLE"
#define RGB_WHITE "WHITE"

// --- Food Level Constants ---
#define FOOD_LEVEL_FULL "FULL"
#define FOOD_LEVEL_HALF "HALF"
#define FOOD_LEVEL_EMPTY "EMPTY"
#define FOOD_LEVEL_UNKNOWN "UNKNOWN"

// --- Bowl Status Constants ---
#define BOWL_STATUS_EMPTY "EMPTY"
#define BOWL_STATUS_PARTIAL "PARTIAL"
#define BOWL_STATUS_FULL "FULL"
#define BOWL_STATUS_UNKNOWN "UNKNOWN"

// --- Feeding Status Constants ---
#define FEEDING_STATUS_READY "READY"
#define FEEDING_STATUS_WAIT "WAIT"
#define FEEDING_STATUS_BOWL_FULL "BOWL_FULL"
#define FEEDING_STATUS_DAILY_LIMIT "DAILY_LIMIT"

// --- Default Settings ---
#define DEFAULT_REFILL_MODE false
#define DEFAULT_AUTO_FEEDING_ENABLED true
#define DEFAULT_SCHEDULED_FEEDING_MODE false
#define DEFAULT_WEIGHT_BASED_FEEDING true

// --- Button Configuration ---
#define DEBOUNCE_DELAY 50

// --- JSON Buffer Sizes ---
#define JSON_BUFFER_SIZE_SMALL 512
#define JSON_BUFFER_SIZE_MEDIUM 1024
#define JSON_BUFFER_SIZE_LARGE 2048

// --- Array Size Macro ---
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// Struct Definitions
struct SystemState
{
  bool initialized = false;
  bool rtcReady = false;
  bool backendConnected = false;
  bool powerOn = false;
  bool refillMode = DEFAULT_REFILL_MODE;
  bool dispensing = false;
  bool autoFeedingEnabled = DEFAULT_AUTO_FEEDING_ENABLED;
  bool scheduledFeedingMode = DEFAULT_SCHEDULED_FEEDING_MODE;
  bool weightBasedFeeding = DEFAULT_WEIGHT_BASED_FEEDING;
  bool animalDetected = false;
  bool bowlEmpty = false;
  bool bowlFull = false;
};

struct ButtonState
{
  bool button1 = false;
  bool button2 = false;
  bool lastButton1 = false;
  bool lastButton2 = false;
};

struct SensorData
{
  float distance = 0.0;
  float distanceInches = 0.0;
  float weight = 0.0;
  float previousWeight = 0.0;
  float weightDifference = 0.0;
  float totalFoodDispensed = 0.0;
  float dailyFoodDispensed = 0.0;
  String foodLevel = FOOD_LEVEL_UNKNOWN;
  String bowlStatus = BOWL_STATUS_UNKNOWN;
  String feedingStatus = FEEDING_STATUS_READY;
};

struct Timing
{
  unsigned long lastButton1Press = 0;
  unsigned long lastButton2Press = 0;
  unsigned long dispenseStartTime = 0;
  unsigned long lastDistanceRead = 0;
  unsigned long lastPIRRead = 0;
  unsigned long lastWeightRead = 0;
  unsigned long lastWeightChange = 0;
  unsigned long lastDailyReset = 0;
  unsigned long lastFeedingTime = 0;
  unsigned long lastRTCRead = 0;
  unsigned long lastDataSync = 0;
  unsigned long lastLCDUpdate = 0;
  unsigned long lastLowFoodBeep = 0;
};

struct TimeData
{
  DateTime lastAutoFeedTime;
  DateTime nextScheduledFeed;
  String currentTimeString = "";
  String nextFeedTimeString = "";
};

struct Endpoints
{
  String hostName = String(PHP_SERVER_URL);
  String feedPath = "/feed.php";
  String statusPath = "/status.php";
  String sensorPath = "/collect_sensor_data.php";
  String commandPath = "/remote_commands.php";
};

#endif // CONFIG_H
