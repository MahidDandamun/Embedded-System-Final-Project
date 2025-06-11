#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <RTClib.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Global object declarations
extern LiquidCrystal_I2C lcd;
extern RTC_DS3231 rtc;
extern Servo myServo;
extern HX711 scale;

// Global variable declarations
extern int feedingTimes[];
extern int numFeedingTimes;
extern SystemState feederSystem;
extern ButtonState buttons;
extern SensorData sensors;
extern Timing timing;
extern TimeData timeData;
extern Endpoints endpoints;

#endif