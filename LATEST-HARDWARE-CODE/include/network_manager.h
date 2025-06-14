#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "config.h"
#include "globals.h"
#include "time_manager.h"

// MQTT function declarations
void setupMQTT();
bool connectMQTT();                                                       // Add this declaration
void handleMQTTCallback(char *topic, byte *payload, unsigned int length); // Add this declaration
void sendSensorDataToAzure();

// Backend communication functions
void handleBackendCommunication();
bool sendHTTPRequest(String path, String queryString);
bool sendFeedingDataToPHP(String feedingType, float amount, String timestamp);
bool sendStatusDataToPHP();
bool sendSensorDataToPHP();
bool checkForRemoteCommands();

#endif