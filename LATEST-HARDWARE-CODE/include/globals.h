#ifndef GLOBALS_H
#define GLOBALS_H

#include <ESP32Servo.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "config.h" // This now includes Arduino.h and RTClib.h

// Global object declarations
extern LiquidCrystal_I2C lcd;
extern ThreeWire myWire;
extern RtcDS1302<ThreeWire> rtc;
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
extern IoTHubConfig iotHub;
extern PubSubClient mqttClient;

// Add this structure definition before the existing structures
struct BowlData
{
  float weight;
  int status;
};

// Add bowl status constants
#define BOWL_EMPTY 0
#define BOWL_LOW 1
#define BOWL_FULL 2
#define BOWL_ERROR 3

// Add bowlData to the extern declarations
extern BowlData bowlData;

#endif