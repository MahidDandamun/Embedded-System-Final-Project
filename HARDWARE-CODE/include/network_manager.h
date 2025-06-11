#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "config.h"
#include "globals.h"
#include "time_manager.h" // Add this line to access formatDateTime and formatTime

void handleBackendCommunication();
bool sendHTTPRequest(String path, String queryString);
bool sendFeedingDataToPHP(String feedingType, float amount, String timestamp);
bool sendStatusDataToPHP();
bool sendSensorDataToPHP();
bool checkForRemoteCommands();

#endif