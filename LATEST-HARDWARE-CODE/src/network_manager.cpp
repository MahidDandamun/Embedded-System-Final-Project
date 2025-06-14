#include "network_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Use constants from config.h instead of hardcoded values
const char *mqttServer = MQTT_SERVER;
const int mqttPort = MQTT_PORT;
const char *deviceId = DEVICE_ID;
const char *sasToken = SAS_TOKEN;
const char *mqttUsername = MQTT_USERNAME;
const char *databaseEndpoint = DATABASE_ENDPOINT;

// Global MQTT objects
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

void setupMQTT()
{
  wifiClient.setInsecure(); // For testing only - use proper certs in production
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(handleMQTTCallback);
  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);

  connectMQTT();
}

bool connectMQTT()
{
  if (mqttClient.connected())
    return true;

  Serial.println("Connecting to Azure IoT Hub...");
  if (mqttClient.connect(deviceId, mqttUsername, sasToken))
  {
    Serial.println("Connected to Azure IoT Hub");
    String methodTopic = "$iothub/methods/POST/#";
    mqttClient.subscribe(methodTopic.c_str());
    return true;
  }

  Serial.println("Failed to connect to Azure IoT Hub");
  return false;
}

void handleMQTTCallback(char *topic, byte *payload, unsigned int length)
{
  // Handle incoming messages
  String message = String((char *)payload, length);
  Serial.printf("Message received on topic: %s\n", topic);
}

void sendSensorDataToAzure()
{
  Serial.println("=== Attempting to send data to Azure IoT Hub ===");

  if (!mqttClient.connected() && !connectMQTT())
  {
    Serial.println("ERROR: Failed to connect to MQTT broker");
    return;
  }

  StaticJsonDocument<1024> doc;

  // Generate UUID
  char id[37];
  snprintf(id, sizeof(id), "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
           random(0, 0xffff), random(0, 0xffff), random(0, 0xffff),
           random(0, 0xffff), random(0, 0xffff), random(0, 0xffff),
           random(0, 0xffff), random(0, 0xffff));

  // Required fields matching DB schema
  doc["id"] = id;
  doc["bowl_weight"] = sensors.weight;
  doc["container_level"] = String(sensors.foodLevel);
  doc["pet_status"] = feederSystem.animalDetected;
  doc["distance_cm"] = sensors.distance;
  doc["daily_food_dispensed"] = sensors.dailyFoodDispensed;
  doc["timestamp"] = feederSystem.rtcReady ? formatDateTime(rtc.now()) : "";

  char payload[1024];
  size_t len = serializeJson(doc, payload, sizeof(payload));

  // Print the JSON payload being sent
  Serial.println("JSON Payload:");
  Serial.println(payload);
  Serial.printf("Payload size: %d bytes\n", len);

  // Send to Azure IoT Hub
  String topic = "devices/" + String(deviceId) + "/messages/events/";
  Serial.printf("Publishing to topic: %s\n", topic.c_str());

  if (mqttClient.publish(topic.c_str(), payload, len))
  {
    Serial.println("✓ Data sent to Azure IoT Hub successfully");
    feederSystem.mqttConnected = true;
  }
  else
  {
    Serial.println("✗ Failed to send data to Azure IoT Hub");
    feederSystem.mqttConnected = false;
  }
  Serial.println("=== End of Azure IoT Hub transmission ===\n");
}

void handleBackendCommunication()
{
  unsigned long currentMillis = millis();
  if (currentMillis - timing.lastDataSync >= DATA_SYNC_INTERVAL)
  {
    // Send data to Azure IoT Hub (this is working)
    sendSensorDataToAzure();

    // ALSO send directly to database (add this)
    sendStatusDataToPHP();

    checkForRemoteCommands();

    timing.lastDataSync = currentMillis;
  }

  // Handle MQTT loop
  if (mqttClient.connected())
  {
    mqttClient.loop();
  }
}

bool sendStatusDataToPHP()
{
  Serial.println("=== Attempting to send status data to PHP backend ===");

  String queryString = "?device_name=" + String(DEVICE_NAME);
  queryString += "&timestamp=" + (feederSystem.rtcReady ? formatDateTime(rtc.now()) : "");
  queryString += "&current_time=" + (feederSystem.rtcReady ? formatTime(rtc.now()) : "");
  queryString += "&bowl_weight=" + String(sensors.weight, 1);
  queryString += "&food_level=" + String(sensors.foodLevel);
  queryString += "&bowl_status=" + String(sensors.bowlStatus);
  queryString += "&daily_food_dispensed=" + String(sensors.dailyFoodDispensed, 1);
  queryString += "&total_food_dispensed=" + String(sensors.totalFoodDispensed, 1);
  queryString += "&auto_feeding_enabled=" + String(feederSystem.autoFeedingEnabled ? "1" : "0");
  queryString += "&scheduled_feeding_mode=" + String(feederSystem.scheduledFeedingMode ? "1" : "0");
  queryString += "&refill_mode=" + String(feederSystem.refillMode ? "1" : "0");
  queryString += "&dispensing=" + String(feederSystem.dispensing ? "1" : "0");
  queryString += "&animal_detected=" + String(feederSystem.animalDetected ? "1" : "0");
  queryString += "&distance_cm=" + String(sensors.distance, 1);
  queryString += "&feeding_status=" + String(sensors.feedingStatus);
  queryString += "&weight_based_feeding=" + String(feederSystem.weightBasedFeeding ? "1" : "0");
  queryString += "&next_feed_time=" + String(timeData.nextFeedTimeString);
  queryString += "&last_feeding_minutes_ago=" + String((millis() - timing.lastFeedingTime) / 60000);
  queryString += "&uptime_seconds=" + String(millis() / 1000);
  queryString += "&wifi_connected=" + String(WiFi.status() == WL_CONNECTED ? "1" : "0");
  queryString += "&wifi_rssi=" + String(WiFi.RSSI());
  queryString += "&rtc_initialized=" + String(feederSystem.rtcReady ? "1" : "0");
  queryString += "&backend_connected=" + String(feederSystem.backendConnected ? "1" : "0");

  Serial.printf("Query String: %s\n", queryString.c_str());

  bool result = sendHTTPRequest("/status", queryString);

  if (result)
  {
    Serial.println("✓ Status data sent to PHP backend successfully");
  }
  else
  {
    Serial.println("✗ Failed to send status data to PHP backend");
  }

  Serial.println("=== End of PHP backend transmission ===\n");
  return result;
}

bool sendFeedingDataToPHP(String feedingType, float amount, String timestamp)
{
  String queryString = "?device_name=" + String(DEVICE_NAME);
  queryString += "&feeding_type=" + feedingType;
  queryString += "&amount_grams=" + String(amount, 1);
  queryString += "&timestamp=" + timestamp;
  queryString += "&bowl_weight=" + String(sensors.weight, 1);
  queryString += "&food_level=" + String(sensors.foodLevel);   // Convert char array to String
  queryString += "&bowl_status=" + String(sensors.bowlStatus); // Convert char array to String
  queryString += "&daily_total=" + String(sensors.dailyFoodDispensed, 1);
  queryString += "&session_total=" + String(sensors.totalFoodDispensed, 1);

  return sendHTTPRequest("/feed", queryString);
}

bool sendSensorDataToPHP()
{
  String queryString = "?device_name=" + String(DEVICE_NAME);
  queryString += "&timestamp=" + (feederSystem.rtcReady ? formatDateTime(rtc.now()) : "");
  return sendHTTPRequest("/sensor", queryString);
}

bool checkForRemoteCommands()
{
  // Implementation for checking remote commands
  return true;
}

bool sendHTTPRequest(String path, String queryString)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("ERROR: WiFi not connected");
    return false;
  }

  HTTPClient http;
  String fullURL = String(DATABASE_ENDPOINT) + path + queryString;

  Serial.printf("Full URL: %s\n", fullURL.c_str());

  http.begin(fullURL);
  http.setTimeout(10000); // 10 second timeout

  int httpResponseCode = http.GET();

  Serial.printf("HTTP Response Code: %d\n", httpResponseCode);

  if (httpResponseCode > 0)
  {
    String response = http.getString();
    Serial.printf("Response: %s\n", response.c_str());
  }

  bool success = (httpResponseCode == 200);
  feederSystem.backendConnected = success;

  http.end();
  return success;
}