#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "config.h"
#include "globals.h"

void handleSensors();
float readUltrasonicDistance();
String getFoodLevel(float distanceInches);
String getBowlStatus(float currentWeight);

#endif