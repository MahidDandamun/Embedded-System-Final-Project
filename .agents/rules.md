# Agent Rules — Pet Feeder Project

## Project Context

This is an open-source automated pet feeder system built with:

- **Hardware:** ESP32 (PlatformIO/Arduino framework)
- **Cloud:** Azure IoT Hub (MQTT) + REST API backend
- **Mobile App:** React Native (Expo) — separate repository
- **Web Flasher:** Next.js + Web Serial API — separate repository
- **PCB:** KiCad
- **3D Model:** Fusion 360

## Coding Standards

### C++ (Firmware)

- Use `const char*` over `String` for fixed strings to reduce heap fragmentation
- Use `char[]` in structs, not `String`
- All timing via `millis()` — never use blocking `delay()` in main loop
- Prefix private/static helpers with `static`
- Use `snprintf` over `sprintf` — always pass buffer size
- Use `strncpy` over `strcpy` — always pass size
- No dynamic memory allocation in loop
- Keep ISRs minimal
- Follow the existing module pattern: each feature in its own `.h` + `.cpp` pair

### Secrets & Security

- **NEVER** commit `secrets.h`, `.env`, API keys, tokens, or passwords
- Always load credentials from `secrets.h` which is gitignored
- Use `secrets.h.template` as the checked-in reference
- For production: use proper TLS certificate pinning (not `setInsecure()`)

### Git

- Conventional commits: `feat:`, `fix:`, `refactor:`, `docs:`, `chore:`
- One logical change per commit
- Do not commit build artifacts (`.pio/`, `build/`, `node_modules/`)
- Do not commit IDE configs (`.vscode/` settings that are user-specific)

## Architecture Decisions

1. **BLE for mobile app** — Low-latency local control without requiring WiFi. The mobile app connects directly to the ESP32 over BLE.
2. **MQTT for cloud** — Reliable pub/sub for telemetry and remote commands via Azure IoT Hub.
3. **HTTP POST for DB** — Simple REST fallback for database status updates.
4. **DS1302 RTC** — Keeps feeding schedule accurate even without WiFi.
5. **Modular firmware** — Each subsystem (sensors, feeding, display, network, BLE) is isolated in its own module for testability and readability.
