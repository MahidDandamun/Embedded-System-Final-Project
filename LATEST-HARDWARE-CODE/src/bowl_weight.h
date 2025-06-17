#ifndef BOWL_WEIGHT_H
#define BOWL_WEIGHT_H

#include <Arduino.h>
#include "HX711.h"

// Load Cell Configuration
#define LOADCELL_DOUT_PIN 5      // Data pin for HX711 - ADJUST THIS PIN AS NEEDED
#define LOADCELL_SCK_PIN 23      // Clock pin for HX711 - ADJUST THIS PIN AS NEEDED
#define CALIBRATION_FACTOR 49400 // Calibration factor for 1kg load cell - ADJUST AS NEEDED
#define WEIGHT_READINGS 3        // Number of readings to average

// Define bowl weight thresholds (in grams)
#define BOWL_EMPTY_THRESHOLD 10.0
#define BOWL_LOW_THRESHOLD 50.0
#define BOWL_MEDIUM_THRESHOLD 100.0

// Function declarations
void setupLoadCell();
float getBowlWeight();
String getBowlWeightStatus();
void tareLoadCell(); // Reset to zero

#endif // BOWL_WEIGHT_H
