# Firmware — Automated Pet Feeder (ESP32)

PlatformIO-based firmware for the ESP32 pet feeder system.

## Features

- Scheduled auto-feeding (4 times/day, configurable)
- Manual feed via physical button or mobile app (BLE)
- Ultrasonic food-level sensing
- HX711 load cell for bowl weight
- PIR motion detection (animal presence)
- RGB LED + buzzer status feedback
- Azure IoT Hub telemetry via MQTT
- REST API push to backend database
- BLE interface for mobile app control & live status

## Quick Start

### 1. Install PlatformIO

```bash
pip install platformio
# or install the VS Code PlatformIO extension
```

### 2. Configure Secrets

```bash
cp include/secrets.h.template include/secrets.h
```

Edit `include/secrets.h` with your actual credentials:
- WiFi SSID & password
- Azure IoT Hub connection string
- Backend API endpoint

> **Never commit `secrets.h`** — it's listed in `.gitignore`.

### 3. Build & Upload

```bash
pio run                 # compile
pio run -t upload       # flash to ESP32
pio device monitor      # serial monitor (115200 baud)
```

### 4. Calibration

Run the calibration test to set your load cell factor:

```bash
pio test -e esp32dev -f test_calibration
```

Or upload `test/test_calibration.cpp` as the main sketch, follow the serial prompts, and note the factor. Update `CALIBRATION_FACTOR` in `include/config.h`.

## Pin Mapping (PCB v2)

| Function        | GPIO |
|-----------------|------|
| Power Relay     | 27   |
| Button 1        | 15   |
| Button 2        | 0    |
| Ultrasonic Trig | 32   |
| Ultrasonic Echo | 33   |
| PIR Sensor      | 25   |
| HX711 DOUT      | 5    |
| HX711 SCK       | 23   |
| RGB Red         | 13   |
| RGB Green       | 12   |
| RGB Blue        | 14   |
| Buzzer          | 26   |
| Servo           | 4    |
| RTC IO          | 16   |
| RTC SCLK        | 17   |
| RTC CE          | 2    |

## Architecture

```
firmware/
├── include/
│   ├── config.h              # All pin/timing/threshold config + structs
│   ├── secrets.h.template    # Template for credentials
│   ├── globals.h             # Extern declarations
│   ├── system_init.h
│   ├── sensor_manager.h
│   ├── feeding_control.h
│   ├── button_handler.h
│   ├── display_manager.h
│   ├── time_manager.h
│   ├── network_manager.h
│   ├── ble_manager.h         # BLE for mobile app
│   └── load_cell.h
├── src/
│   ├── main.cpp              # Setup + main loop
│   ├── globals.cpp
│   ├── system_init.cpp
│   ├── sensor_manager.cpp
│   ├── feeding_control.cpp
│   ├── button_handler.cpp
│   ├── display_manager.cpp
│   ├── time_manager.cpp
│   ├── network_manager.cpp   # MQTT + HTTP
│   ├── ble_manager.cpp       # BLE service
│   └── load_cell.cpp
├── test/
│   └── test_calibration.cpp  # Load cell calibration
└── platformio.ini
```

## BLE Protocol (for Mobile App)

| Characteristic | UUID | Direction | Format |
|----------------|------|-----------|--------|
| Command        | `beb5483e-...` | Write | `{"cmd":"feed"}` or `{"cmd":"refill"}` or `{"cmd":"tare"}` |
| Status         | `a3c87500-...` | Notify | JSON with all sensor/state data |

The mobile app connects via BLE, sends commands, and receives real-time status updates every 2 seconds.
