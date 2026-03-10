#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "config.h"
#include "globals.h"

void handleSensors();
float readUltrasonicDistance();
const char* getFoodLevel(float distanceCm);
const char* getBowlStatus(float currentWeight);
void updateBowlWeight();

#endif // SENSOR_MANAGER_H
