# Automated Pet Feeder System

An open-source, IoT-enabled automated pet feeder built with ESP32. Schedule feedings, monitor your pet's eating habits in real-time, and control everything from your phone.

<!-- ![Pet Feeder Hero](docs/images/hero.png) -->

## Features

- **Scheduled Auto-Feeding** — 4 configurable daily feeding times
- **Manual Feed** — Physical button or remote trigger from mobile app
- **Food Level Monitoring** — Ultrasonic sensor detects container level (FULL / HALF / EMPTY)
- **Bowl Weight Tracking** — HX711 load cell measures food in the bowl
- **Pet Detection** — PIR motion sensor detects animal presence
- **Mobile App Control** — React Native app connects via Bluetooth (BLE)
- **Cloud Telemetry** — Azure IoT Hub (MQTT) + REST API for data logging
- **Web Flasher** — Flash firmware from your browser via USB — no IDE needed
- **Status Display** — 16x2 LCD + RGB LED + buzzer feedback
- **Safety Limits** — Daily max (200g), minimum interval (5 min), bowl-full detection

## Repository Structure

```
├── firmware/               # ESP32 firmware (PlatformIO / C++)
│   ├── include/            #   Header files + config
│   ├── src/                #   Source files
│   ├── test/               #   Calibration test
│   └── platformio.ini
├── PCB/                    # KiCad PCB design for breakout board
├── 3D-MODEL/               # Fusion 360 housing + component mounts
├── docs/                   # Specs for mobile app & web flasher
│   ├── MOBILE_APP_SPEC.md  #   React Native mobile app spec
│   └── WEB_FLASHER_SPEC.md #   Next.js web flasher spec
├── .agents/                # AI agent rules, workflows, context
└── README.md               # You are here
```

### Companion Repositories (Separate)

| Repo | Tech Stack | Purpose |
|------|-----------|---------|
| `petfeeder-mobile` | React Native (Expo) + TypeScript | Mobile app — BLE control, dashboard, history |
| `petfeeder-web` | Next.js + Web Serial API | Browser-based firmware flasher & calibration |

## Hardware Requirements

| Component | Model | Qty |
|-----------|-------|-----|
| Microcontroller | ESP32 DevKit C | 1 |
| Ultrasonic Sensor | HC-SR04 | 1 |
| Load Cell + Amplifier | 5kg load cell + HX711 | 1 |
| PIR Motion Sensor | Generic | 1 |
| Servo Motor | SG90 or MG90S | 1 |
| LCD Display | 16x2 I2C (address 0x27) | 1 |
| RTC Module | DS1302 | 1 |
| RGB LED | Common cathode | 1 |
| Active Buzzer | 5V | 1 |
| Push Buttons | IP65 12mm momentary (x2) | 2 |
| Power Supply | 5V 2A, 5.5x2.1mm barrel jack | 1 |
| Rocker Switch | SPST | 1 |
| Custom PCB | See `PCB/` directory | 1 |
| 3D-Printed Housing | See `3D-MODEL/` directory | 1 |

## Quick Start

### 1. Clone

```bash
git clone https://github.com/MahidDandamun/Embedded-System-Final-Project.git
cd Embedded-System-Final-Project
```

### 2. Configure Secrets

```bash
cd firmware
cp include/secrets.h.template include/secrets.h
```

Edit `firmware/include/secrets.h` with your credentials:

```c
#define ENV_WIFI_SSID       "YourWiFi"
#define ENV_WIFI_PASSWORD   "YourPassword"
#define ENV_MQTT_SERVER     "your-hub.azure-devices.net"
#define ENV_MQTT_PORT       8883
#define ENV_DEVICE_ID       "petfeeder"
#define ENV_SAS_TOKEN       "SharedAccessSignature sr=..."
#define ENV_MQTT_USERNAME   "your-hub.azure-devices.net/petfeeder/?api-version=2021-04-12"
#define ENV_DATABASE_ENDPOINT "https://your-api.azurewebsites.net/api/devices/status"
```

> **Never commit `secrets.h`** — it's in `.gitignore`.

### 3. Build & Flash

```bash
# Install PlatformIO CLI
pip install platformio

# Build
pio run

# Upload to ESP32
pio run -t upload

# Monitor serial output
pio device monitor
```

### 4. Calibrate Load Cell

```bash
# Upload calibration firmware
pio test -e esp32dev -f test_calibration
```

Follow the serial prompts to find your `CALIBRATION_FACTOR`, then update it in `firmware/include/config.h`.

### 5. Assemble Hardware

1. Print the housing from `3D-MODEL/` (see [3D-MODEL/README.md](3D-MODEL/README.md))
2. Solder or order the PCB from `PCB/` (see [PCB/README.md](PCB/README.md))
3. Wire everything per the [pin mapping](firmware/README.md#pin-mapping-pcb-v2)
4. Power on and verify via serial monitor

## Architecture

```
┌─────────────┐     BLE      ┌─────────────┐
│  Mobile App │◄────────────►│    ESP32     │
│ (React Native)             │  Firmware    │
└─────────────┘              │             │
                             │  Sensors:    │
┌─────────────┐    MQTT      │  - Ultrasonic│
│  Azure IoT  │◄────────────►│  - Load Cell │
│    Hub      │              │  - PIR       │
└──────┬──────┘              │  - RTC       │
       │                     │             │
       │ REST                │  Actuators:  │
       ▼                     │  - Servo     │
┌─────────────┐              │  - LCD       │
│  Backend DB │              │  - RGB LED   │
│  (REST API) │              │  - Buzzer    │
└─────────────┘              └──────┬───────┘
                                    │ USB
┌─────────────┐    Web Serial      │
│  Web Flasher│◄───────────────────┘
│  (Next.js)  │
└─────────────┘
```

## Mobile App

The companion mobile app (separate repo) provides:

- **Dashboard** — Real-time sensor data, feeding status, pet detection
- **Feeding History** — Charts and timeline of all feeding events
- **Schedule Control** — Edit feeding times and portion sizes
- **BLE Connection** — Direct Bluetooth communication with the feeder
- **Push Notifications** — Low food alerts, missed feeding warnings

**Tech Stack:** React Native (Expo) + TypeScript + Zustand + react-native-ble-plx

See full spec: [docs/MOBILE_APP_SPEC.md](docs/MOBILE_APP_SPEC.md)

## Web Flasher

A browser-based tool to flash firmware and calibrate sensors — no IDE installation required:

1. Connect ESP32 via USB-C
2. Enter WiFi/cloud credentials in the web form
3. Click "Flash" — firmware uploads directly from Chrome
4. Run guided sensor calibration

**Tech Stack:** Next.js + TypeScript + Web Serial API + esptool-js

See full spec: [docs/WEB_FLASHER_SPEC.md](docs/WEB_FLASHER_SPEC.md)

## Pin Mapping

| Function | GPIO | Notes |
|----------|------|-------|
| Power Relay | 27 | System power control |
| Button 1 (Refill) | 15 | INPUT_PULLUP |
| Button 2 (Feed) | 0 | INPUT_PULLUP |
| Ultrasonic Trig | 32 | |
| Ultrasonic Echo | 33 | |
| PIR Sensor | 25 | |
| HX711 DOUT | 5 | |
| HX711 SCK | 23 | |
| RGB Red | 13 | |
| RGB Green | 12 | |
| RGB Blue | 14 | |
| Buzzer | 26 | Active buzzer |
| Servo | 4 | SG90/MG90S |
| RTC IO | 16 | DS1302 |
| RTC SCLK | 17 | DS1302 |
| RTC CE | 2 | DS1302 |

## Security Notes

- All credentials are stored in `secrets.h` which is **never committed** to git
- A `secrets.h.template` is provided as reference
- Azure SAS tokens should be rotated regularly
- For production: replace `setInsecure()` with proper TLS certificate pinning
- If you ever accidentally commit secrets: **rotate ALL credentials immediately**

### Regarding Credentials in Git History

The credentials found in older commits have been **revoked and are no longer valid**. The WiFi password, Azure SAS tokens, and API endpoints from the git history are all invalidated. Always generate your own fresh credentials.

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feat/amazing-feature`)
3. Use conventional commits (`feat:`, `fix:`, `docs:`, `refactor:`)
4. Ensure no secrets are in your commits
5. Open a Pull Request

## License

This project is licensed under the [MIT License](LICENSE).

## Authors

- Irish Micole Cando
- Jelisha Bugnon
- Angela Corpuz
- Mahid Dandamun
- Miguel Alfonso Neri

---

> Built as a final project for Embedded Systems course. Designed to be open-source, educational, and production-ready.
