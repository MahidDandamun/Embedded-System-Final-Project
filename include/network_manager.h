#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "config.h"
#include "globals.h"
#include "time_manager.h"

// MQTT function declarations
void setupMQTT();
bool connectMQTT();
void handleMQTTCallback(char *topic, byte *payload, unsigned int length);
void sendSensorDataToAzure();
void handleBackendCommunication();
bool checkForRemoteCommands();
void setupTime();
void handleDirectMethod(char *topic, byte *payload, unsigned int length);
void processMQTTLoop();
bool sendToDatabase();
#endif