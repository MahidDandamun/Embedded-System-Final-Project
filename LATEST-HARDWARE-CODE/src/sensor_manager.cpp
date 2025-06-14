#include "sensor_manager.h"
#include "feeding_control.h" // Add this include to access getFeedingStatus()

void handleSensors()
{
  unsigned long currentMillis = millis();

  // Read ultrasonic sensor
  if (currentMillis - timing.lastUltrasonicRead >= ULTRASONIC_READ_INTERVAL)
  {
    sensors.distance = readUltrasonicDistance();
    String level = getFoodLevel(sensors.distance);
    strcpy(sensors.foodLevel, level.c_str()); // Copy String to char array
    timing.lastUltrasonicRead = currentMillis;
  }

  // Read weight sensor
  if (currentMillis - timing.lastWeightRead >= WEIGHT_READ_INTERVAL)
  {
    if (scale.is_ready())
    {
      sensors.weight = scale.get_units(SCALE_READINGS);
      String status = getBowlStatus(sensors.weight);
      strcpy(sensors.bowlStatus, status.c_str()); // Copy String to char array
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
  String status = getFeedingStatus();
  strcpy(sensors.feedingStatus, status.c_str()); // Copy String to char array
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
    return String(FOOD_LEVEL_FULL);

  if (distanceInches <= FOOD_HALF_DISTANCE)
    return String(FOOD_LEVEL_HALF);

  return String(FOOD_LEVEL_EMPTY);
}

String getBowlStatus(float currentWeight)
{
  if (currentWeight <= EMPTY_BOWL_THRESHOLD)
    return String(BOWL_STATUS_EMPTY);

  if (currentWeight >= FULL_BOWL_THRESHOLD)
    return String(BOWL_STATUS_FULL);

  return String(BOWL_STATUS_PARTIAL);
}

void updateLoadCellReading()
{
  if (scale.is_ready())
  {
    float weight = scale.get_units(10); // average over 10 readings
    sensors.weight = weight;
    Serial.printf("Load Cell Weight: %.2f grams\n", weight);

    // Optional: Set bowl status based on weight
    if (weight < 10.0)
    {
      strcpy(sensors.bowlStatus, "Empty");
    }
    else if (weight < 50.0)
    {
      strcpy(sensors.bowlStatus, "Low");
    }
    else
    {
      strcpy(sensors.bowlStatus, "OK");
    }
  }
  else
  {
    Serial.println("HX711 not ready.");
    sensors.weight = -1;
    strcpy(sensors.bowlStatus, "Error");
  }
}
