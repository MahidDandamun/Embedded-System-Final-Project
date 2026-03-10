# 3D Model — Pet Feeder Housing

This directory contains the complete 3D model files for the automated pet feeder enclosure.

## Files

| File | Description |
|------|-------------|
| `AUTOMATED PET FEEDER SYSTEM 3D MODEL.f3z` | Full Fusion 360 project (editable) |
| `PRAMS 2.0 v3.step` | Complete assembled housing (STEP) |
| `FINAL PROJ v5.step` | Final assembly with all components |
| `Pet bowls v1.step` | Food/water bowl models |
| `microservoAssembly v2.step` | Servo motor mount assembly |
| `ESP32-DevkitC32D v1.step` | ESP32 dev board model |
| `HC-SR04 (1) v1.step` | Ultrasonic sensor mount |
| `LCD WITH IIC v1.step` | LCD display bracket |
| `Led RGB for arduino v1.step` | RGB LED holder |
| `load sensor FHDA v1.step` | Load cell mount |
| `PIR Sensor (1) v1.step` | PIR motion sensor mount |
| `RTC DS1302 real time clock v1.step` | RTC module bracket |
| `DC Power Jack 5.5mm x 2.1mm v1.step` | Power jack cutout |
| `RockerSwitch v1.step` | Power switch mount |
| `STARELO 12mm Momentary Push Button...v1.step` | Button mount |

## Housing Assembly Guide

### What You Need

- 3D printer (FDM, PLA or PETG recommended)
- M3 screws and nuts
- Hot glue gun (for sensor mounts)
- Wire (22 AWG for internal wiring)

### Print Settings

| Parameter | Value |
|-----------|-------|
| Material | PLA or PETG |
| Layer Height | 0.2mm |
| Infill | 20–30% |
| Supports | Yes (for overhangs) |
| Wall Thickness | 1.2mm (3 walls) |

### Assembly Steps

1. **Print all housing parts** — Use the STEP files to slice in your slicer software (Cura, PrusaSlicer, etc.)

2. **Install the load cell** — Mount the load cell under the bowl platform using M3 screws. Run wires through the channel to the electronics bay.

3. **Mount the servo** — Use `microservoAssembly v2.step` as reference. The servo controls the food gate. Secure with provided mounting holes.

4. **Install sensors**
   - **Ultrasonic (HC-SR04):** Mount at the top of the food container, pointing downward
   - **PIR Sensor:** Mount near the bowl at pet height
   - **RGB LED:** Install in the front-facing LED slot

5. **Install electronics**
   - **ESP32:** Snap into the bracket inside the electronics bay
   - **LCD Display:** Slide into the front-facing LCD slot
   - **RTC Module:** Mount near the ESP32
   - **Power Jack & Switch:** Install in the rear panel cutouts

6. **Wire everything** per the pin mapping in [firmware/README.md](../firmware/README.md)

7. **Close the housing** and secure with M3 screws

### System Preview

<!-- Add a screenshot of your 3D model here -->
<!-- ![Pet Feeder 3D Model](images/3d-model-preview.png) -->

> **TODO:** Add photos/screenshots of the assembled 3D model here.

### Tips

- Print the bowl platform in PETG for better food safety
- Use cable management clips inside the housing
- Leave access to the USB port for firmware updates
- The power jack hole fits a standard 5.5mm × 2.1mm barrel connector
