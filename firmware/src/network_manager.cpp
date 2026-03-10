#include "network_manager.h"
#include "feeding_control.h"
#include "sensor_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// MQTT client
static WiFiClientSecure wifiClient;
static PubSubClient mqttClient(wifiClient);

// ============================================================================
// MQTT Setup & Connection
// ============================================================================
void setupMQTT() {
    setupTime();
    wifiClient.setInsecure(); // TODO: add root CA for production

    mqttClient.setServer(iotConfig.mqttServer, iotConfig.mqttPort);
    mqttClient.setCallback(handleMQTTCallback);
    mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
    mqttClient.setKeepAlive(120);

    mqttClient.disconnect();
    delay(100);

    Serial.println("Setting up MQTT...");
    if (connectMQTT()) {
        Serial.println("MQTT ready");
    } else {
        Serial.println("MQTT setup failed — will retry in loop");
    }
}

bool connectMQTT() {
    const int maxRetries = 3;

    for (int i = 0; i < maxRetries && !mqttClient.connected(); i++) {
        Serial.printf("MQTT connect attempt %d/%d\n", i + 1, maxRetries);

        if (mqttClient.connect(iotConfig.deviceId,
                               iotConfig.mqttUsername,
                               iotConfig.sasToken)) {
            Serial.println("Connected to Azure IoT Hub");
            mqttClient.subscribe("$iothub/methods/POST/#");
            mqttClient.subscribe("$iothub/twin/PATCH/properties/desired/#");
            feederSystem.mqttConnected = true;
            return true;
        }

        Serial.printf("MQTT state: %d\n", mqttClient.state());
        delay(3000 * (i + 1));
    }

    feederSystem.mqttConnected = false;
    return false;
}

// ============================================================================
// Backend Communication Loop
// ============================================================================
void handleBackendCommunication() {
    if (WiFi.status() != WL_CONNECTED) return;

    unsigned long now = millis();

    // Keep MQTT alive
    if (mqttClient.connected()) {
        mqttClient.loop();
    } else if (now - timing.lastMQTTReconnect >= MQTT_RECONNECT_INTERVAL) {
        Serial.println("MQTT reconnecting...");
        connectMQTT();
        timing.lastMQTTReconnect = now;
    }

    // Periodic sensor/status sync
    if (now - timing.lastDataSync >= DATA_SYNC_INTERVAL) {
        sendSensorDataToAzure();
        sendToDatabase();
        timing.lastDataSync = now;
    }
}

// ============================================================================
// Data Transmission
// ============================================================================
bool sendSensorDataToAzure() {
    if (!mqttClient.connected()) return false;

    StaticJsonDocument<512> doc;
    doc["deviceId"]       = iotConfig.deviceId;
    doc["distance"]       = sensors.distance;
    doc["weight"]         = sensors.weight;
    doc["foodLevel"]      = sensors.foodLevel;
    doc["bowlStatus"]     = sensors.bowlStatus;
    doc["motionDetected"] = sensors.motionDetected;
    doc["animalDetected"] = feederSystem.animalDetected;
    doc["dispensing"]     = feederSystem.dispensing;
    doc["dailyFood"]      = sensors.dailyFoodDispensed;
    doc["totalFood"]      = sensors.totalFoodDispensed;
    doc["feedingStatus"]  = sensors.feedingStatus;
    doc["refillMode"]     = feederSystem.refillMode;
    doc["uptime"]         = millis() / 1000;
    doc["freeHeap"]       = ESP.getFreeHeap();
    doc["rssi"]           = WiFi.RSSI();

    if (feederSystem.rtcReady) {
        RtcDateTime now = rtc.GetDateTime();
        doc["timestamp"] = formatDateTime(now);
        doc["nextFeed"]  = timeData.nextFeedTimeString;
    }

    char payload[512];
    serializeJson(doc, payload, sizeof(payload));

    bool ok = mqttClient.publish(iotConfig.telemetryTopic, payload);
    if (ok) {
        Serial.println("Telemetry sent");
    } else {
        Serial.println("Telemetry send failed");
    }
    return ok;
}

bool sendFeedingDataToAzure(const char* feedingType, float amount,
                             const char* timestamp) {
    if (!mqttClient.connected()) return false;

    StaticJsonDocument<256> doc;
    doc["deviceId"]     = iotConfig.deviceId;
    doc["event"]        = "feeding";
    doc["feedingType"]  = feedingType;
    doc["amountGrams"]  = amount;
    doc["timestamp"]    = timestamp;
    doc["bowlWeight"]   = sensors.weight;
    doc["foodLevel"]    = sensors.foodLevel;
    doc["dailyTotal"]   = sensors.dailyFoodDispensed;

    char payload[256];
    serializeJson(doc, payload, sizeof(payload));

    return mqttClient.publish(iotConfig.telemetryTopic, payload);
}

bool sendToDatabase() {
    if (WiFi.status() != WL_CONNECTED) return false;

    HTTPClient http;
    http.setTimeout(5000);
    http.begin(iotConfig.databaseEndpoint);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<512> doc;
    doc["deviceId"]      = iotConfig.deviceId;
    doc["firmware"]      = FIRMWARE_VERSION;
    doc["distance"]      = sensors.distance;
    doc["weight"]        = sensors.weight;
    doc["foodLevel"]     = sensors.foodLevel;
    doc["bowlStatus"]    = sensors.bowlStatus;
    doc["dailyFood"]     = sensors.dailyFoodDispensed;
    doc["motionDetected"]= sensors.motionDetected;
    doc["dispensing"]    = feederSystem.dispensing;
    doc["refillMode"]    = feederSystem.refillMode;
    doc["rssi"]          = WiFi.RSSI();
    doc["uptime"]        = millis() / 1000;

    if (feederSystem.rtcReady) {
        RtcDateTime now = rtc.GetDateTime();
        doc["timestamp"] = formatDateTime(now);
    }

    char payload[512];
    serializeJson(doc, payload, sizeof(payload));

    int code = http.POST(payload);
    http.end();

    bool ok = (code >= 200 && code < 300);
    feederSystem.backendConnected = ok;
    return ok;
}

// ============================================================================
// MQTT Callback
// ============================================================================
void handleMQTTCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr(topic);
    String message((char*)payload, length);

    Serial.printf("MQTT << %s : %s\n", topic, message.c_str());

    if (topicStr.startsWith("$iothub/methods/POST/")) {
        handleDirectMethod(topic, payload, length);
    }
}

void handleDirectMethod(char* topic, byte* payload, unsigned int length) {
    String topicStr(topic);
    String message((char*)payload, length);

    // Extract method name: $iothub/methods/POST/{methodName}/?$rid={rid}
    int methodStart = topicStr.indexOf("POST/") + 5;
    int methodEnd   = topicStr.indexOf("/?");
    String method   = topicStr.substring(methodStart, methodEnd > 0 ? methodEnd : topicStr.length());

    // Extract request ID
    int ridStart = topicStr.indexOf("$rid=");
    String rid   = ridStart >= 0 ? topicStr.substring(ridStart + 5) : "";

    Serial.printf("Direct method: %s (rid=%s)\n", method.c_str(), rid.c_str());

    String responseTopic = "$iothub/methods/res/200/?$rid=" + rid;
    StaticJsonDocument<128> resp;

    if (method == "feed") {
        if (canDispenseFood() && !feederSystem.dispensing) {
            performManualFeed();
            resp["status"] = "ok";
            resp["message"] = "Feeding started";
        } else {
            resp["status"] = "error";
            resp["message"] = sensors.feedingStatus;
        }
    } else if (method == "status") {
        resp["foodLevel"]  = sensors.foodLevel;
        resp["bowlStatus"] = sensors.bowlStatus;
        resp["weight"]     = sensors.weight;
        resp["dispensing"]  = feederSystem.dispensing;
    } else {
        responseTopic = "$iothub/methods/res/404/?$rid=" + rid;
        resp["error"] = "Unknown method";
    }

    char buf[128];
    serializeJson(resp, buf, sizeof(buf));
    mqttClient.publish(responseTopic.c_str(), buf);
}
