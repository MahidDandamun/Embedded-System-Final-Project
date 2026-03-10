#include "ble_manager.h"
#include "feeding_control.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

static BLEServer* pServer = nullptr;
static BLECharacteristic* pCmdChar = nullptr;
static BLECharacteristic* pStatusChar = nullptr;
static bool deviceConnected = false;
static unsigned long lastBLEUpdate = 0;
static const unsigned long BLE_UPDATE_INTERVAL = 2000;

// ============================================================================
// BLE Callbacks
// ============================================================================
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* server) override {
        deviceConnected = true;
        feederSystem.bleConnected = true;
        Serial.println("BLE client connected");
    }

    void onDisconnect(BLEServer* server) override {
        deviceConnected = false;
        feederSystem.bleConnected = false;
        Serial.println("BLE client disconnected");
        server->startAdvertising();
    }
};

class CmdCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
        String value = pChar->getValue().c_str();
        Serial.printf("BLE cmd: %s\n", value.c_str());

        StaticJsonDocument<128> doc;
        DeserializationError err = deserializeJson(doc, value);
        if (err) {
            Serial.println("BLE JSON parse error");
            return;
        }

        const char* cmd = doc["cmd"] | "";

        if (strcmp(cmd, "feed") == 0) {
            if (canDispenseFood() && !feederSystem.dispensing) {
                performManualFeed();
            }
        } else if (strcmp(cmd, "refill") == 0) {
            feederSystem.refillMode = !feederSystem.refillMode;
        } else if (strcmp(cmd, "tare") == 0) {
            extern void tareLoadCell();
            tareLoadCell();
        }
    }
};

// ============================================================================
// BLE Init
// ============================================================================
void initBLE() {
    BLEDevice::init(BLE_DEVICE_NAME);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    BLEService* pService = pServer->createService(BLE_SERVICE_UUID);

    // Command characteristic (write)
    pCmdChar = pService->createCharacteristic(
        BLE_CHAR_CMD_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    pCmdChar->setCallbacks(new CmdCallbacks());

    // Status characteristic (notify)
    pStatusChar = pService->createCharacteristic(
        BLE_CHAR_STATUS_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pStatusChar->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising* pAdv = BLEDevice::getAdvertising();
    pAdv->addServiceUUID(BLE_SERVICE_UUID);
    pAdv->setScanResponse(true);
    pAdv->setMinPreferred(0x06);
    BLEDevice::startAdvertising();

    Serial.println("BLE advertising started");
}

// ============================================================================
// BLE Loop — push status to connected mobile app
// ============================================================================
void handleBLE() {
    if (!deviceConnected) return;

    unsigned long now = millis();
    if (now - lastBLEUpdate < BLE_UPDATE_INTERVAL) return;
    lastBLEUpdate = now;

    sendBLEStatus();
}

void sendBLEStatus() {
    if (!deviceConnected || pStatusChar == nullptr) return;

    StaticJsonDocument<256> doc;
    doc["food"]       = sensors.foodLevel;
    doc["bowl"]       = sensors.bowlStatus;
    doc["weight"]     = sensors.weight;
    doc["distance"]   = sensors.distance;
    doc["dispensing"]  = feederSystem.dispensing;
    doc["refill"]     = feederSystem.refillMode;
    doc["dailyFood"]  = sensors.dailyFoodDispensed;
    doc["motion"]     = feederSystem.animalDetected;
    doc["status"]     = sensors.feedingStatus;
    doc["wifi"]       = WiFi.status() == WL_CONNECTED;
    doc["mqtt"]       = feederSystem.mqttConnected;

    if (feederSystem.rtcReady) {
        doc["time"]     = timeData.currentTimeString;
        doc["nextFeed"] = timeData.nextFeedTimeString;
    }

    char buf[256];
    size_t len = serializeJson(doc, buf, sizeof(buf));
    pStatusChar->setValue((uint8_t*)buf, len);
    pStatusChar->notify();
}
