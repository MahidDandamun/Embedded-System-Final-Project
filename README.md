

---

# Automated Pet Feeder System

This project is the final requirement for our Embedded Systems course. The goal is to design and implement an **automated pet feeder system** using the ESP32 microcontroller, a suite of sensors, and remote cloud-based control.

## Project Overview

The Automated Pet Feeder System enables pet owners to schedule and monitor feedings for their pets. The system combines hardware, embedded programming, and a web-based interface for seamless, remote operation and data logging.

## Features

- **Automated Feeding:** Scheduled or on-demand feeding for your pets.
- **Remote Control:** Manage and trigger feedings through a web app from anywhere.
- **Data Logging:** Monitor feeding times, quantities, and other statistics for health tracking.
- **All-in-One Repo:** Includes 3D models, hardware source code, PCB designs, and interfacing diagrams.

## Getting Started

### Prerequisites

- **Hardware:** ESP32 development board, appropriate sensors (weight, proximity, etc.), actuators (motor/servo for dispenser), and supporting electronics.
- **Software:** 
  - Arduino IDE or PlatformIO for ESP32 code
  - Required ESP32 and sensor libraries
  - Web development libraries (see relevant subdirectories)
- **Cloud Service:** (Configure your preferred IoT/cloud provider for remote control)

### Cloning the Repository

```bash
git clone https://github.com/MahidDandamun/Embedded-System-Final-Project.git
```

### Repository Structure

- `/hardware` - Embedded source code for ESP32 and sensor interfacing
- `/webapp` - Web application for remote control and monitoring
- `/pcb` - PCB design files
- `/3d-models` - 3D printable parts and enclosure files
- `/docs` - Wiring and interfacing diagrams

### Building & Usage

1. Assemble hardware as per the PCB and interfacing diagrams.
2. Upload the ESP32 firmware using Arduino IDE/PlatformIO.
3. Set up the web app (instructions in `/webapp`).
4. Connect the ESP32 to Wi-Fi and configure cloud service credentials.
5. Use the web interface to control and monitor the pet feeder.

## Dependencies

- ESP32 board libraries (see `hardware/README.md`)
- Sensor and actuator libraries for ESP32 (e.g., Servo, WiFi, HTTP, etc.)
- Node.js or web stack dependencies for the web app (see `/webapp`)

## Authors

- Irish Micole Cando
- Jelisha Bugnon
- Angela Corpuz
- Mahid Dandamun
- Miguel Alfonso Neri

## License

This project is licensed under the [MIT License](LICENSE).

---
