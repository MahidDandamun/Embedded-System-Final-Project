#include "sensor_manager.h"

void handleSensors()
{
  unsigned long currentMillis = millis();

  // Read ultrasonic sensor
  if (currentMillis - timing.lastUltrasonicRead >= ULTRASONIC_READ_INTERVAL)
  {
    sensors.distance = readUltrasonicDistance();
    sensors.foodLevel = getFoodLevel(sensors.distance);
    timing.lastUltrasonicRead = currentMillis;
  }

  // Read weight sensor
  if (currentMillis - timing.lastWeightRead >= WEIGHT_READ_INTERVAL)
  {
    if (scale.is_ready())
    {
      sensors.weight = scale.get_units(SCALE_READINGS);
      sensors.bowlStatus = getBowlStatus(sensors.weight);
    }
    timing.lastWeightRead = currentMillis;
  }

  // Read PIR sensor
  sensors.motionDetected = digitalRead(PIR_PIN);
  if (sensors.motionDetected)
  {
    feederSystem.animalDetected = true;
    timing.lastMotionTime = currentMillis;
  }
  else if (currentMillis - timing.lastMotionTime > PIR_TIMEOUT)
  {
    feederSystem.animalDetected = false;
  }

  // Update feeding status
  sensors.feedingStatus = getFeedingStatus();
}

float readUltrasonicDistance()
{
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  float distanceCm = duration * 0.034 / 2;
  float distanceInches = distanceCm / 2.54;

  return distanceInches;
}

String getFoodLevel(float distanceInches)
{
  if (distanceInches <= FOOD_FULL_DISTANCE)
    return FOOD_LEVEL_FULL;

  if (distanceInches <= FOOD_HALF_DISTANCE)
    return FOOD_LEVEL_HALF;

  return FOOD_LEVEL_EMPTY;
}

String getBowlStatus(float currentWeight)
{
  if (currentWeight <= EMPTY_BOWL_THRESHOLD)
    return BOWL_STATUS_EMPTY;

  if (currentWeight >= FULL_BOWL_THRESHOLD)
    return BOWL_STATUS_FULL;

  return BOWL_STATUS_PARTIAL;
}