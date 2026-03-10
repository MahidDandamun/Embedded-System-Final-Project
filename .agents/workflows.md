# Agent Workflows

## Firmware Development Workflow

```
1. Edit code in firmware/src/ and firmware/include/
2. Build: pio run
3. Upload: pio run -t upload
4. Monitor: pio device monitor
5. Test: pio test
```

## Adding a New Sensor

```
1. Create include/new_sensor.h with function declarations
2. Create src/new_sensor.cpp with implementation
3. Add extern globals to include/globals.h if needed
4. Add pin definitions to include/config.h
5. Call init from system_init.cpp
6. Call handler from main.cpp loop
7. Add data fields to SensorData struct in config.h
8. Update BLE status payload in ble_manager.cpp
9. Update MQTT telemetry payload in network_manager.cpp
```

## Adding a New BLE Command

```
1. Add command handler in ble_manager.cpp CmdCallbacks::onWrite()
2. Send JSON from mobile app: {"cmd": "your_command", ...params}
3. Document in firmware/README.md BLE Protocol table
```

## Adding a New MQTT Direct Method

```
1. Add method handler in network_manager.cpp handleDirectMethod()
2. Register in Azure IoT Hub portal
3. Send response on "$iothub/methods/res/{status}/?$rid={rid}"
```

## Calibration Workflow

```
1. Upload test/test_calibration.cpp
2. Follow serial prompts to tare and place known weight
3. Note the CALIBRATION_FACTOR
4. Update CALIBRATION_FACTOR in include/config.h
5. Re-upload main firmware
```

## Secrets Management

```
1. cp include/secrets.h.template include/secrets.h
2. Edit secrets.h with real values
3. NEVER commit secrets.h
4. If accidentally committed: rotate ALL credentials immediately
```

## Release Workflow

```
1. Update FIRMWARE_VERSION in config.h
2. Test all features (feeding, BLE, MQTT, sensors)
3. Run calibration verification
4. Commit with: feat: release v{version}
5. Tag: git tag v{version}
6. Push: git push --tags
```
