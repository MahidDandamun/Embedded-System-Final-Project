# Agent Context — Project Overview

## What Is This?

An open-source **Automated Pet Feeder System** built as an embedded systems project. It consists of multiple repositories and components:

### This Repository (Main)
- **firmware/** — ESP32 C++ firmware (PlatformIO)
- **PCB/** — KiCad breakout board design
- **3D-MODEL/** — Fusion 360 housing and component mounts
- **docs/** — Additional documentation

### Companion Repositories (Separate)
- **petfeeder-mobile** — React Native (Expo) mobile app
- **petfeeder-web** — Next.js web flasher + calibration tool

## Hardware Components

| Component | Model | Purpose |
|-----------|-------|---------|
| Microcontroller | ESP32 DevKit C | Main controller, WiFi + BLE |
| Ultrasonic Sensor | HC-SR04 | Food level detection |
| Load Cell + HX711 | Generic 5kg | Bowl weight measurement |
| PIR Sensor | Generic | Animal presence detection |
| Servo Motor | SG90 / MG90S | Food gate actuator |
| LCD Display | 16×2 I2C (0x27) | Status display |
| RTC Module | DS1302 | Timekeeping |
| RGB LED | Common cathode | Status indicator |
| Buzzer | Active | Audio feedback |
| Push Buttons | x2 (IP65) | Refill mode + Manual feed |
| Power | 5V 2A barrel jack | System power |

## Communication Channels

1. **BLE** — Mobile app ↔ ESP32 (local, low-latency)
2. **MQTT** — ESP32 → Azure IoT Hub (telemetry + remote commands)
3. **HTTP** — ESP32 → REST API backend (database sync)
4. **Web Serial** — PC ↔ ESP32 (firmware flashing via web app)

## Feeding Schedule

Default: 08:00, 12:00, 18:00, 22:00 — configurable in `config.h`.

- 25g per feeding
- 200g daily max
- 5-minute minimum interval between feedings
- Bowl-full detection prevents over-feeding

## Team

- Irish Micole Cando
- Jelisha Bugnon
- Angela Corpuz
- Mahid Dandamun
- Miguel Alfonso Neri
