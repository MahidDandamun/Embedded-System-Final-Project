#include "sensor_manager.h"
#include "feeding_control.h"

void handleSensors() {
    unsigned long now = millis();

    // Ultrasonic distance
    if (now - timing.lastUltrasonicRead >= ULTRASONIC_READ_INTERVAL) {
        sensors.distance = readUltrasonicDistance();
        const char* level = getFoodLevel(sensors.distance);
        strncpy(sensors.foodLevel, level, sizeof(sensors.foodLevel) - 1);
        timing.lastUltrasonicRead = now;
    }

    // PIR motion
    sensors.motionDetected = digitalRead(PIR_PIN);
    if (sensors.motionDetected) {
        feederSystem.animalDetected = true;
        timing.lastMotionTime = now;
    } else if (now - timing.lastMotionTime > PIR_TIMEOUT) {
        feederSystem.animalDetected = false;
    }

    // Feeding status string
    const char* status = getFeedingStatus();
    strncpy(sensors.feedingStatus, status, sizeof(sensors.feedingStatus) - 1);
}

float readUltrasonicDistance() {
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

    long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000); // 30ms timeout
    if (duration == 0) return 999.0; // no echo

    float distanceCm = duration * 0.034f / 2.0f;
    return distanceCm;
}

const char* getFoodLevel(float distanceCm) {
    if (distanceCm <= FOOD_FULL_DISTANCE)  return FOOD_LEVEL_FULL;
    if (distanceCm <= FOOD_HALF_DISTANCE)  return FOOD_LEVEL_HALF;
    return FOOD_LEVEL_EMPTY;
}

const char* getBowlStatus(float currentWeight) {
    if (currentWeight <= EMPTY_BOWL_THRESHOLD) return BOWL_STATUS_EMPTY;
    if (currentWeight >= FULL_BOWL_THRESHOLD)  return BOWL_STATUS_FULL;
    return BOWL_STATUS_PARTIAL;
}

void updateBowlWeight() {
    if (scale.is_ready()) {
        sensors.weight = scale.get_units(SCALE_READINGS);
        if (sensors.weight < 0) sensors.weight = 0;
        const char* status = getBowlStatus(sensors.weight);
        strncpy(sensors.bowlStatus, status, sizeof(sensors.bowlStatus) - 1);
        feederSystem.bowlFull = (strcmp(sensors.bowlStatus, BOWL_STATUS_FULL) == 0);
    }
}
