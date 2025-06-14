#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#include "config.h"
#include "globals.h"
#include "time_manager.h" // Add this line for time functions

void systemStart();
void initializeLCD();
void initializeRTC();
void initializePins();
void initializeWiFi();
void initializeSensors();
void setupMQTT(); // Add this declaration

#endif