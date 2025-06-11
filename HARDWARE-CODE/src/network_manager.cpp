#include "network_manager.h"

void handleBackendCommunication()
{
  unsigned long currentMillis = millis();
  if (currentMillis - timing.lastDataSync >= DATA_SYNC_INTERVAL)
  {
    sendStatusDataToPHP();
    sendSensorDataToPHP();
    checkForRemoteCommands();

    timing.lastDataSync = currentMillis;
  }
}

bool sendHTTPRequest(String path, String queryString)
{
  if (WiFi.status() != WL_CONNECTED)
    return false;

  HTTPClient http;
  String fullURL = endpoints.hostName + path + queryString;
  http.begin(fullURL);

  int httpResponseCode = http.GET();
  bool success = (httpResponseCode == HTTP_CODE_OK);

  if (success && (path == endpoints.statusPath || path == endpoints.feedPath))
  {
    feederSystem.backendConnected = true;
  }
  else if (path == endpoints.statusPath || path == endpoints.feedPath)
  {
    feederSystem.backendConnected = false;
  }

  http.end();
  return success;
}

bool sendFeedingDataToPHP(String feedingType, float amount, String timestamp)
{
  String queryString = "?device_name=" + String(DEVICE_NAME);
  queryString += "&feeding_type=" + feedingType;
  queryString += "&amount_grams=" + String(amount, 1);
  queryString += "&timestamp=" + timestamp;
  queryString += "&bowl_weight=" + String(sensors.weight, 1);
  queryString += "&food_level=" + sensors.foodLevel;
  queryString += "&bowl_status=" + sensors.bowlStatus;
  queryString += "&daily_total=" + String(sensors.dailyFoodDispensed, 1);
  queryString += "&session_total=" + String(sensors.totalFoodDispensed, 1);

  return sendHTTPRequest(endpoints.feedPath, queryString);
}

bool sendStatusDataToPHP()
{
  String queryString = "?device_name=" + String(DEVICE_NAME);
  queryString += "&timestamp=" + (feederSystem.rtcReady ? formatDateTime(rtc.now()) : "");
  queryString += "&current_time=" + (feederSystem.rtcReady ? formatTime(rtc.now()) : "");
  queryString += "&bowl_weight=" + String(sensors.weight, 1);
  queryString += "&food_level=" + sensors.foodLevel;
  queryString += "&bowl_status=" + sensors.bowlStatus;
  queryString += "&daily_food_dispensed=" + String(sensors.dailyFoodDispensed, 1);
  queryString += "&total_food_dispensed=" + String(sensors.totalFoodDispensed, 1);
  queryString += "&auto_feeding_enabled=" + String(feederSystem.autoFeedingEnabled ? "1" : "0");
  queryString += "&scheduled_feeding_mode=" + String(feederSystem.scheduledFeedingMode ? "1" : "0");
  queryString += "&refill_mode=" + String(feederSystem.refillMode ? "1" : "0");
  queryString += "&dispensing=" + String(feederSystem.dispensing ? "1" : "0");
  queryString += "&animal_detected=" + String(feederSystem.animalDetected ? "1" : "0");
  queryString += "&distance_cm=" + String(sensors.distance, 1);
  queryString += "&feeding_status=" + sensors.feedingStatus;
  queryString += "&weight_based_feeding=" + String(feederSystem.weightBasedFeeding ? "1" : "0");
  queryString += "&next_feed_time=" + timeData.nextFeedTimeString;
  queryString += "&last_feeding_minutes_ago=" + String((millis() - timing.lastFeedingTime) / 60000);
  queryString += "&uptime_seconds=" + String(millis() / 1000);
  queryString += "&wifi_connected=" + String(WiFi.status() == WL_CONNECTED ? "1" : "0");
  queryString += "&wifi_rssi=" + String(WiFi.RSSI());
  queryString += "&rtc_initialized=" + String(feederSystem.rtcReady ? "1" : "0");
  queryString += "&backend_connected=" + String(feederSystem.backendConnected ? "1" : "0");

  return sendHTTPRequest(endpoints.statusPath, queryString);
}

bool sendSensorDataToPHP()
{
  String queryString = "?device_name=" + String(DEVICE_NAME);
  queryString += "&timestamp=" + (feederSystem.rtcReady ? formatDateTime(rtc.now()) : "");
  // Add additional sensor data as needed
  return sendHTTPRequest(endpoints.sensorPath, queryString);
}

bool checkForRemoteCommands()
{
  // Implementation for checking remote commands
  // This function appears to be incomplete in your original code
  return true;
}