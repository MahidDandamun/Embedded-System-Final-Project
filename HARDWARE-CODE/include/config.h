#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define WIFI_RETRY_ATTEMPTS 20

// Device Configuration
#define DEVICE_NAME "PetFeeder_001"

// Pin Definitions
#define POWER_PIN 2
#define BUTTON1_PIN 14
#define BUTTON2_PIN 27
#define ULTRASONIC_TRIG_PIN 5
#define ULTRASONIC_ECHO_PIN 18
#define PIR_PIN 19
#define HX711_DOUT_PIN 16
#define HX711_SCK_PIN 4
#define RED_PIN 25
#define GREEN_PIN 26
#define BLUE_PIN 33
#define BUZZER_PIN 32
#define SERVO_PIN 13

// LCD Configuration
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Timing Intervals (in milliseconds)
#define ULTRASONIC_READ_INTERVAL 1000
#define WEIGHT_READ_INTERVAL 500
#define RTC_READ_INTERVAL 5000
#define LCD_UPDATE_INTERVAL 1000
#define DATA_SYNC_INTERVAL 30000
#define DISPENSE_TIME 2000
#define MIN_FEEDING_INTERVAL 300000 // 5 minutes
#define PIR_TIMEOUT 30000           // 30 seconds
#define DEBOUNCE_DELAY 50

// Sensor Thresholds
#define FOOD_FULL_DISTANCE 2.0    // inches
#define FOOD_HALF_DISTANCE 4.0    // inches
#define EMPTY_BOWL_THRESHOLD 10.0 // grams
#define FULL_BOWL_THRESHOLD 200.0 // grams
#define SCALE_READINGS 10

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

// HTTP Status Codes
#define HTTP_CODE_OK 200

// Utility Macros
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

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
  float weight = 0.0;
  bool motionDetected = false;
  String foodLevel = FOOD_LEVEL_EMPTY;
  String bowlStatus = BOWL_STATUS_EMPTY;
  String feedingStatus = "Ready";
  float dailyFoodDispensed = 0.0;
  float totalFoodDispensed = 0.0;
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
};

struct TimeData
{
  String currentTimeString = "";
  String nextFeedTimeString = "";
  DateTime lastAutoFeedTime;
  DateTime nextScheduledFeed;
};

struct Endpoints
{
  String hostName = "http://your-server.com";
  String statusPath = "/pet_feeder/status.php";
  String feedPath = "/pet_feeder/feed.php";
  String sensorPath = "/pet_feeder/sensor.php";
};

#endif