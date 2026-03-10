# PCB — Pet Feeder Breakout Board

KiCad PCB design for the ESP32 pet feeder breakout/carrier board.

## Files

| File | Description |
|------|-------------|
| `FINAL PROJ.kicad_sch` | Schematic |
| `FINAL PROJ.kicad_pcb` | PCB layout |
| `FINAL PROJ.kicad_pro` | KiCad project |
| `FINAL PROJ.step` | 3D model of PCB |
| `freerouting.dsn` | Auto-routing export |
| `config.h` | Pin mapping reference (mirrors firmware config) |

## Features

- ESP32 DevKit C socket
- Breakout headers for all sensors (HC-SR04, PIR, HX711, DS1302)
- Servo connector (5V + signal)
- I2C header for LCD
- RGB LED + buzzer outputs
- 5V barrel jack power input with rocker switch
- Dual push-button inputs

## Opening the Design

1. Install [KiCad](https://www.kicad.org/) (v7+ recommended)
2. Open `FINAL PROJ.kicad_pro`
3. Schematic → `FINAL PROJ.kicad_sch`
4. PCB Layout → `FINAL PROJ.kicad_pcb`

## Manufacturing

Export Gerbers from KiCad and upload to your preferred PCB fab (JLCPCB, PCBWay, OSH Park, etc.).
