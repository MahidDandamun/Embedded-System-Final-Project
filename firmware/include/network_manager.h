#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "config.h"
#include "globals.h"
#include "time_manager.h"

// WiFi / MQTT lifecycle
void setupMQTT();
bool connectMQTT();
void handleBackendCommunication();

// Data transmission
bool sendToDatabase();
bool sendSensorDataToAzure();
bool sendFeedingDataToAzure(const char* feedingType, float amount, const char* timestamp);

// MQTT callbacks
void handleMQTTCallback(char* topic, byte* payload, unsigned int length);
void handleDirectMethod(char* topic, byte* payload, unsigned int length);

#endif // NETWORK_MANAGER_H
