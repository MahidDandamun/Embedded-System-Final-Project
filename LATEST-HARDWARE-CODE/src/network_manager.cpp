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
  String topicStr = String(topic);

  Serial.printf("Message received on topic: %s\n", topic);
  Serial.printf("Message content: %s\n", message.c_str());

  // Check if this is a direct method call
  if (topicStr.startsWith("$iothub/methods/POST/"))
  {
    Serial.println("Direct method detected - routing to handleDirectMethod");
    handleDirectMethod(topic, payload, length);
  }
  // Check if this is a device twin update
  else if (topicStr.startsWith("$iothub/twin/PATCH/properties/desired/"))
  {
    Serial.println("Device twin update received");
    // Handle device twin updates here if needed
  }
  else
  {
    Serial.println("Unknown message type received");
  }
}

bool sendToDatabase()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("ERROR: WiFi not connected");
    return false;
  }

  // Update LCD to show database connection attempt
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Testing Database");
  lcd.setCursor(0, 1);
  lcd.print("Connection...");
  Serial.println("Testing database connection...");

  HTTPClient https;
  https.setTimeout(3000); // Reduced to 3 second timeout
  https.begin("https://petfeeder-embedded.azurewebsites.net/api/devices/status");

  https.addHeader("Content-Type", "application/json");
  https.addHeader("Accept", "application/json");

  // Fix payload format - use simpler structure to avoid 400 error
  StaticJsonDocument<256> doc;
  doc["deviceId"] = String(DEVICE_ID);

  // Use simpler timestamp format
  if (feederSystem.rtcReady)
  {
    RtcDateTime now = getPhilippineTime();
    doc["timestamp"] = formatDateTime(now);
  }
  else
  {
    doc["timestamp"] = String(millis());
  }

  // Ensure numeric values are properly formatted
  doc["bowlWeight"] = (float)sensors.weight;
  doc["containerLevel"] = (int)sensors.foodLevel;
  doc["petPresent"] = (bool)feederSystem.animalDetected;
  doc["status"] = "online";

  String jsonStr;
  serializeJson(doc, jsonStr);

  Serial.println("Sending HTTP POST to database...");
  Serial.println("JSON Payload: " + jsonStr);

  int httpCode = https.POST(jsonStr);

  // Update LCD with database connection result - reduced delay
  lcd.clear();
  lcd.setCursor(0, 0);
  if (httpCode > 0)
  {
    Serial.printf("Database Response Code: %d\n", httpCode);
    String response = https.getString();
    Serial.printf("Response: %s\n", response.c_str());

    if (httpCode == 200 || httpCode == 201)
    {
      lcd.print("Database: OK");
      Serial.println("✓ Database connection successful");
      feederSystem.backendConnected = true;
    }
    else
    {
      lcd.print("Database: ERROR");
      lcd.setCursor(0, 1);
      lcd.print("Code: " + String(httpCode));
      Serial.printf("✗ Database error with code: %d\n", httpCode);
      Serial.println("Response body: " + response);
      feederSystem.backendConnected = false;
    }
  }
  else
  {
    Serial.printf("✗ HTTP POST failed, error: %s\n", https.errorToString(httpCode).c_str());
    lcd.print("Database: FAIL");
    lcd.setCursor(0, 1);
    lcd.print("Network Error");
    feederSystem.backendConnected = false;
  }

  https.end();
  delay(750); // Reduced from 1500 to 750ms

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

  StaticJsonDocument<512> doc; // Reduced size

  // Simplified payload structure
  doc["deviceId"] = String(DEVICE_ID);
  doc["timestamp"] = feederSystem.rtcReady ? formatDateTime(getPhilippineTime()) : String(millis());
  doc["bowlWeight"] = (float)sensors.weight;
  doc["containerLevel"] = (int)sensors.foodLevel;
  doc["petPresent"] = (bool)feederSystem.animalDetected;
  doc["messageType"] = "telemetry";

  char payload[512];
  size_t len = serializeJson(doc, payload, sizeof(payload));

  String topic = String("devices/") + DEVICE_ID + "/messages/events/";

  Serial.println("JSON Payload:");
  serializeJsonPretty(doc, Serial);
  Serial.printf("\nPayload size: %d bytes\n", len);
  Serial.printf("Publishing to topic: %s\n", topic.c_str());

  if (mqttClient.publish(topic.c_str(), payload, len))
  {
    Serial.println("✓ Data sent to Azure IoT Hub successfully");
    feederSystem.mqttConnected = true;
    feederSystem.backendConnected = true;
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
  Serial.printf("Payload length: %d\n", length);

  // Print the payload for debugging
  String payloadStr = "";
  for (int i = 0; i < length; i++)
  {
    payloadStr += (char)payload[i];
  }
  Serial.println("Payload: " + payloadStr);

  String topicStr = String(topic);
  Serial.println("Full topic string: " + topicStr);

  int methodStart = topicStr.indexOf("POST/") + 5;
  int methodEnd = topicStr.indexOf("/", methodStart);

  Serial.printf("methodStart: %d, methodEnd: %d\n", methodStart, methodEnd);

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
      Serial.println("Processing runMotors command...");

      // Use the exact same dispensing logic as manual button press
      if (!feederSystem.dispensing)
      {
        // Call the EXACT same function as manual button press
        Serial.println("Starting remote feeding sequence (using manual dispensing logic)...");
        performManualFeed(); // Use the same function as manual button
        Serial.println("Remote motors sequence executed successfully");

        // Send success response
        responseTopic = "$iothub/methods/res/200/?$rid=" + requestId;
        responsePayload = "{\"status\":\"success\",\"message\":\"Motors started successfully\",\"dispensing\":true}";
      }
      else
      {
        // Cannot dispense food - send appropriate error
        String reason = "Motors already running";
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

    // Publish the response immediately
    Serial.println("Sending response...");
    Serial.println("Response Topic: " + responseTopic);
    Serial.println("Response Payload: " + responsePayload);

    bool published = mqttClient.publish(responseTopic.c_str(), responsePayload.c_str());
    if (published)
    {
      Serial.println("✓ Response published successfully");

      // Force immediate transmission
      mqttClient.loop();
      delay(100); // Small delay to ensure message is sent
      mqttClient.loop();
    }
    else
    {
      Serial.println("✗ Failed to publish response!");
      Serial.printf("MQTT State: %d\n", mqttClient.state());

      // Try to reconnect and send again
      if (!mqttClient.connected())
      {
        Serial.println("MQTT disconnected, attempting reconnection...");
        if (connectMQTT())
        {
          Serial.println("Reconnected, trying to send response again...");
          mqttClient.publish(responseTopic.c_str(), responsePayload.c_str());
          mqttClient.loop();
        }
      }
    }
  }
  else
  {
    Serial.println("Could not extract method name properly");
    Serial.printf("methodStart: %d, methodEnd: %d\n", methodStart, methodEnd);

    // Send malformed request response
    int ridStart = topicStr.indexOf("$rid=") + 5;
    String requestId = topicStr.substring(ridStart);
    String responseTopic = "$iothub/methods/res/400/?$rid=" + requestId;
    String responsePayload = "{\"status\":\"error\",\"message\":\"Malformed method request\"}";

    bool published = mqttClient.publish(responseTopic.c_str(), responsePayload.c_str());
    if (published)
    {
      Serial.println("✓ Error response sent for malformed request");
      mqttClient.loop();
    }
    else
    {
      Serial.println("✗ Failed to send error response");
    }
  }

  Serial.println("Direct method processing complete");
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

void processMQTTLoop()
{
  if (mqttClient.connected())
  {
    mqttClient.loop();
  }
  else if (WiFi.status() == WL_CONNECTED)
  {
    // Try to reconnect if WiFi is still connected
    Serial.println("MQTT disconnected, attempting reconnection...");
    connectMQTT();
  }
}