#include "load_cell.h"
#include "soc/rtc.h"
#include <Wire.h>
#include "config.h"
#include "globals.h"

// Load Cell Functions
void setupLoadCell()
{
  // Power control for the load cell if needed
  // Set CPU frequency for better performance
  rtc_cpu_freq_config_t config;
  rtc_clk_cpu_freq_get_config(&config);
  rtc_clk_cpu_freq_to_config(RTC_CPU_FREQ_80M, &config);
  rtc_clk_cpu_freq_set_config_fast(&config);

  // Initialize the scale
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare(); // Reset scale to zero

  // Initialize buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Log that load cell is initialized
  Serial.println("Load cell initialized");

  delay(500); // Give time for the scale to stabilize
}

float getWeight()
{
  // Get weight in grams (more appropriate for 1kg load cell)
  if (scale.is_ready())
  {
    return scale.get_units(SCALE_READINGS);
  }
  else
  {
    return 0.0;
  }
}

void updateBowlWeight()
{
  if (scale.is_ready())
  {
    // Get weight and update global sensor data
    float weight = getWeight();
    sensors.weight = weight;

    // Update bowl status based on weight
    if (weight < EMPTY_BOWL_THRESHOLD)
    {
      strcpy(sensors.bowlStatus, BOWL_STATUS_EMPTY);
    }
    else if (weight < FULL_BOWL_THRESHOLD)
    {
      strcpy(sensors.bowlStatus, BOWL_STATUS_PARTIAL);
    }
    else
    {
      strcpy(sensors.bowlStatus, BOWL_STATUS_FULL);
    }
  }
  else
  {
    sensors.weight = 0;
    strcpy(sensors.bowlStatus, "ERROR");
  }
}

void playBuzzer(int beepCount, int beepDuration, int pauseDuration)
{
  for (int i = 0; i < beepCount; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(beepDuration);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < beepCount - 1) // Don't pause after the last beep
    {
      delay(pauseDuration);
    }
  }
}

void handleButtonPress()
{
  unsigned long currentTime = millis();

  // Read button states (INPUT_PULLUP means LOW when pressed)
  bool button1Pressed = digitalRead(BUTTON1_PIN) == LOW;
  bool button2Pressed = digitalRead(BUTTON2_PIN) == LOW;

  // Handle Button 1 (Dispense) with debouncing
  if (button1Pressed && !buttons.lastButton1 &&
      (currentTime - timing.lastButton1Press) > DEBOUNCE_DELAY)
  {
    timing.lastButton1Press = currentTime;
    playBuzzer(1, BUZZER_SHORT_BEEP, 0); // Single beep for button press

    if (!feederSystem.refillMode && !buttons.manualFeedInProgress)
    {
      buttons.manualFeedInProgress = true;
      dispenseFood();
      buttons.manualFeedInProgress = false;
    }
    else if (feederSystem.refillMode)
    {
      displayMessage("System Paused", "Press Button2");
    }
  }

  // Handle Button 2 (Refill/Pause) with debouncing
  if (button2Pressed && !buttons.lastButton2 &&
      (currentTime - timing.lastButton2Press) > DEBOUNCE_DELAY)
  {
    timing.lastButton2Press = currentTime;
    playBuzzer(2, BUZZER_SHORT_BEEP, BUZZER_SHORT_PAUSE); // Double beep for refill

    // Toggle refill mode
    feederSystem.refillMode = !feederSystem.refillMode;

    if (feederSystem.refillMode)
    {
      strcpy(sensors.feedingStatus, "REFILL MODE");
      displayMessage("REFILL MODE", "System Paused");
      feederSystem.autoFeedingEnabled = false; // Disable auto feeding
    }
    else
    {
      strcpy(sensors.feedingStatus, "Ready");
      displayMessage("System Resumed", "Ready");
      feederSystem.autoFeedingEnabled = true; // Re-enable auto feeding
      delay(2000);                            // Show message for 2 seconds
    }
  }

  // Update button states for next iteration
  buttons.lastButton1 = button1Pressed;
  buttons.lastButton2 = button2Pressed;
}

void dispenseFood()
{
  if (feederSystem.refillMode)
  {
    return; // Don't dispense if in refill mode
  }

  feederSystem.dispensing = true;
  strcpy(sensors.feedingStatus, "Dispensing...");
  displayMessage("Dispensing...", "Please wait");

  // Move servo from 90 to 45 degrees and back, 3 times
  for (int i = 0; i < 3; i++)
  {
    // Move to 45 degrees (left)
    myServo.write(45);
    playBuzzer(1, BUZZER_SHORT_BEEP, 0); // Beep during movement
    delay(500);

    // Return to 90 degrees (center)
    myServo.write(90);
    delay(500);
  }

  // Final beep sequence to indicate completion
  playBuzzer(3, BUZZER_SHORT_BEEP, BUZZER_SHORT_PAUSE);

  // Update sensor data
  sensors.dailyFoodDispensed += FOOD_PORTION_GRAMS;
  sensors.totalFoodDispensed += FOOD_PORTION_GRAMS;

  strcpy(sensors.feedingStatus, "Dispensed");
  displayMessage("Food Dispensed", "Complete");

  delay(2000); // Show completion message

  feederSystem.dispensing = false;
  strcpy(sensors.feedingStatus, "Ready");
}

void displayMessage(String line1, String line2)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (line2.length() > 0)
  {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

void displayWeight(float weight)
{
  if (feederSystem.refillMode)
  {
    lcd.setCursor(0, 0);
    lcd.print("REFILL MODE     ");
    lcd.setCursor(0, 1);
    lcd.print("System Paused   ");
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("Weight:        ");
    lcd.setCursor(0, 1);
    lcd.print(weight, 2);
    lcd.print(" g           ");
  }
}
