#include "globals.h"

// Global object definitions
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
RTC_DS3231 rtc;
Servo myServo;
HX711 scale;

// Global variable definitions
int feedingTimes[] = {FEEDING_TIME_1, FEEDING_TIME_2, FEEDING_TIME_3, FEEDING_TIME_4};
int numFeedingTimes = ARRAY_SIZE(feedingTimes);
SystemState feederSystem;
ButtonState buttons;
SensorData sensors;
Timing timing;
TimeData timeData;
Endpoints endpoints;