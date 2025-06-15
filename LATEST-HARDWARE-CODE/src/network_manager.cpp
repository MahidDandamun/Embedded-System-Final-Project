#include "network_manager.h"
#include "feeding_control.h"
#include "time_manager.h"
#include "sensor_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char *mqttServer = MQTT_SERVER;
const int mqttPort = MQTT_PORT;
const char *deviceId = DEVICE_ID;
const char *sasToken = SAS_TOKEN;
const char *mqttUsername = MQTT_USERNAME;
const char *databaseEndpoint = DATABASE_ENDPOINT;

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

void setupMQTT()
{
  setupTime();
  wifiClient.setInsecure(); // For testing only

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(handleMQTTCallback);
  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
  mqttClient.setKeepAlive(120);

  mqttClient.disconnect();
  delay(100);

  Serial.println("\n=== Setting up MQTT Connection ===");
  if (connectMQTT())
  {
    Serial.println("✓ MQTT Setup Complete");
  }
  else
  {
    Serial.println("✗ MQTT Setup Failed");
  }
  Serial.println("===============================\n");
}

bool connectMQTT()
{
  const int maxRetries = 3;
  int retryCount = 0;

  while (retryCount < maxRetries && !mqttClient.connected())
  {
    Serial.println("Connecting to Azure IoT Hub...");
    Serial.printf("Server: %s\n", mqttServer);
    Serial.printf("Port: %d\n", mqttPort);
    Serial.printf("Username: %s\n", mqttUsername);

    String clientId = String(DEVICE_ID);

    if (mqttClient.connect(clientId.c_str(), mqttUsername, sasToken))
    {
      Serial.println("Connected to Azure IoT Hub");

      mqttClient.subscribe("$iothub/methods/POST/#");
      mqttClient.subscribe("$iothub/twin/PATCH/properties/desired/#");

      feederSystem.mqttConnected = true;
      return true;
    }

    retryCount++;
    Serial.printf("Failed to connect, attempt %d of %d\n", retryCount, maxRetries);
    Serial.printf("MQTT State: %d\n", mqttClient.state());
    delay(5000 * retryCount);
  }

  feederSystem.mqttConnected = false;
  Serial.println("Failed to connect to Azure IoT Hub after all retries");
  return false;
}

void handleMQTTCallback(char *topic, byte *payload, unsigned int length)
{
  String message = String((char *)payload, length);
  Serial.printf("Message received on topic: %s\n", topic);
}

bool sendToDatabase()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("ERROR: WiFi not connected");
    return false;
  }

  HTTPClient https;
  https.begin("https://petfeeder-embedded.azurewebsites.net/api/devices/status");

  https.addHeader("Content-Type", "application/json");
  https.addHeader("Accept", "application/json");

  StaticJsonDocument<512> doc;
  // doc["timestamp"] = feederSystem.rtcReady ? formatDateTime(getPhilippineTime()) : String(millis());
  doc["bowl_weight"] = sensors.weight;
  doc["container_level"] = String(sensors.foodLevel);
  doc["pet_status"] = feederSystem.animalDetected;

  String jsonStr;
  serializeJson(doc, jsonStr);

  Serial.println("Sending HTTP POST to database...");
  Serial.println(jsonStr);

  int httpCode = https.POST(jsonStr);

  if (httpCode > 0)
  {
    Serial.printf("Database Response Code: %d\n", httpCode);
    String response = https.getString();
    Serial.printf("Response: %s\n", response.c_str());
  }
  else
  {
    Serial.printf("✗ HTTP POST failed, error: %s\n", https.errorToString(httpCode).c_str());
  }

  https.end();

  return httpCode == 200 || httpCode == 201;
}

void sendSensorDataToAzure()
{
  Serial.println("\n=== Attempting to send data to Azure IoT Hub ===");

  if (!mqttClient.connected())
  {
    Serial.println("Reconnecting to MQTT...");
    if (!connectMQTT())
    {
      Serial.println("✗ Failed to reconnect to MQTT");
      return;
    }
  }

  StaticJsonDocument<1024> doc;

  char id[37];
  snprintf(id, sizeof(id), "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
           random(0, 0xffff), random(0, 0xffff), random(0, 0xffff),
           random(0, 0xffff), random(0, 0xffff), random(0, 0xffff),
           random(0, 0xffff), random(0, 0xffff));

  JsonObject telemetry = doc.createNestedObject("telemetry");
  telemetry["messageType"] = "deviceTelemetry";
  telemetry["id"] = id;
  telemetry["timestamp"] = feederSystem.rtcReady ? formatDateTime(getPhilippineTime()) : String(millis());
  telemetry["bowl_weight"] = sensors.weight;
  telemetry["container_level"] = String(sensors.foodLevel);
  telemetry["pet_status"] = feederSystem.animalDetected;

  doc["properties"] = "iothub-content-type=application/json";

  char payload[1024];
  size_t len = serializeJson(doc, payload, sizeof(payload));

  String topic = String("devices/") + DEVICE_ID + "/messages/events/$.ct=application%2Fjson&$.ce=utf-8";

  Serial.println("JSON Payload:");
  serializeJsonPretty(doc, Serial);
  Serial.printf("\nPayload size: %d bytes\n", len);
  Serial.printf("Publishing to topic: %s\n", topic.c_str());

  if (mqttClient.publish(topic.c_str(), payload, len))
  {
    Serial.println("✓ Data sent to Azure IoT Hub successfully");
    feederSystem.mqttConnected = true;
    feederSystem.backendConnected = true;

    // Send data to backend database
    sendToDatabase();
  }
  else
  {
    Serial.println("✗ Failed to send data to Azure IoT Hub");
    feederSystem.mqttConnected = false;
    feederSystem.backendConnected = false;
  }

  Serial.println("=== End of Azure IoT Hub transmission ===\n");
}

void handleBackendCommunication()
{
  unsigned long currentMillis = millis();

  if (currentMillis - timing.lastDataSync >= DATA_SYNC_INTERVAL)
  {
    handleSensors();         // ✅ Update sensor values first
    sendSensorDataToAzure(); // ✅ Now you send up-to-date values
    checkForRemoteCommands();
    timing.lastDataSync = currentMillis;
  }

  if (mqttClient.connected())
  {
    mqttClient.loop();
  }
}

void setupTime()
{
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(".");
    yield();
    nowSecs = time(nullptr);
  }
  Serial.println(" synchronized!");
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.printf("Current time: %s", asctime(&timeinfo));
}

bool checkForRemoteCommands()
{
  return true;
}

void handleDirectMethod(char *topic, byte *payload, unsigned int length)
{
  Serial.printf("Direct method received on topic: %s\n", topic);

  String topicStr = String(topic);
  int methodStart = topicStr.indexOf("POST/") + 5;
  int methodEnd = topicStr.indexOf("/", methodStart);

  // Check if we found both markers
  if (methodStart > 4 && methodEnd > methodStart)
  {
    String methodName = topicStr.substring(methodStart, methodEnd);
    Serial.print("Method: ");
    Serial.println(methodName);

    // Extract request ID for response
    int ridStart = topicStr.indexOf("$rid=") + 5;
    String requestId = topicStr.substring(ridStart);

    String responseTopic;
    String responsePayload;

    if (methodName == "runMotors")
    {
      // Check if feeding is possible
      if (canDispenseFood() && !feederSystem.dispensing)
      {
        // Execute the feeding sequence
        handleFeeding();
        Serial.println("Motors sequence executed successfully");

        // Send success response
        responseTopic = "$iothub/methods/res/200/?$rid=" + requestId;
        responsePayload = "{\"status\":\"success\",\"message\":\"Motors started successfully\",\"dispensing\":true}";
      }
      else
      {
        // Cannot dispense food - send appropriate error
        String reason = "";
        if (feederSystem.dispensing)
        {
          reason = "Motors already running";
        }
        else if (!canDispenseFood())
        {
          reason = getFeedingStatus(); // This will give us the specific reason
        }

        Serial.println("Cannot run motors: " + reason);

        // Send error response with specific reason
        responseTopic = "$iothub/methods/res/400/?$rid=" + requestId;
        responsePayload = "{\"status\":\"error\",\"message\":\"Cannot run motors\",\"reason\":\"" + reason + "\"}";
      }
    }
    else
    {
      Serial.println("Unknown method: " + methodName);
      // Send method not found response
      responseTopic = "$iothub/methods/res/404/?$rid=" + requestId;
      responsePayload = "{\"status\":\"error\",\"message\":\"Method not found\"}";
    }

    // Publish the response
    mqttClient.publish(responseTopic.c_str(), responsePayload.c_str());
    Serial.println("Response sent: " + responsePayload);
  }
  else
  {
    Serial.println("Could not extract method name properly");
    Serial.print("methodStart: ");
    Serial.println(methodStart);
    Serial.print("methodEnd: ");
    Serial.println(methodEnd);

    // Send malformed request response
    int ridStart = topicStr.indexOf("$rid=") + 5;
    String requestId = topicStr.substring(ridStart);
    String responseTopic = "$iothub/methods/res/400/?$rid=" + requestId;
    String responsePayload = "{\"status\":\"error\",\"message\":\"Malformed method request\"}";
    mqttClient.publish(responseTopic.c_str(), responsePayload.c_str());
  }
}

bool verifyMessageDelivery()
{
  unsigned long startTime = millis();
  while (millis() - startTime < 5000)
  {
    if (mqttClient.loop())
    {
      return true;
    }
    delay(100);
  }
  return false;
}
