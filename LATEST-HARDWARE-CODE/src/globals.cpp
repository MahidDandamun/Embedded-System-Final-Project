#include "globals.h"

// Global object definitions
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
ThreeWire myWire(RTC_IO, RTC_SCLK, RTC_CE);
RtcDS1302<ThreeWire> rtc(myWire);
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
IoTHubConfig iotHub;