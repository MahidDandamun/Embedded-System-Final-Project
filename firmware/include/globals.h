#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Global hardware objects
extern LiquidCrystal_I2C lcd;
extern ThreeWire myWire;
extern RtcDS1302<ThreeWire> rtc;
extern Servo myServo;
extern HX711 scale;

// Global state
extern int feedingTimes[];
extern int numFeedingTimes;
extern SystemState feederSystem;
extern ButtonState buttons;
extern SensorData sensors;
extern Timing timing;
extern TimeData timeData;
extern IoTHubConfig iotConfig;

#endif // GLOBALS_H
