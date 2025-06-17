#include "button_handler.h"
#include "feeding_control.h"
#include "globals.h"

// Button state variables
bool lastButton1State = HIGH;
bool lastButton2State = HIGH;
unsigned long lastButton1Press = 0;
unsigned long lastButton2Press = 0;
const unsigned long debounceDelay = 300;
void performManualFeed(); // Forward declaration
void initButtons()
{
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  // Read initial states
  lastButton1State = digitalRead(BUTTON1_PIN);
  lastButton2State = digitalRead(BUTTON2_PIN);

  Serial.println("Buttons initialized with INPUT_PULLUP");
  Serial.printf("Button 1 Pin: %d, Initial State: %s\n", BUTTON1_PIN, lastButton1State ? "HIGH (not pressed)" : "LOW (pressed)");
  Serial.printf("Button 2 Pin: %d, Initial State: %s\n", BUTTON2_PIN, lastButton2State ? "HIGH (not pressed)" : "LOW (pressed)");
}

void handleButtons()
{
  // Read current button states
  bool button1State = digitalRead(BUTTON1_PIN);
  bool button2State = digitalRead(BUTTON2_PIN);
  unsigned long currentTime = millis();

  // Debug button states periodically
  static unsigned long lastDebugPrint = 0;
  if (currentTime - lastDebugPrint > 5000) // Every 5 seconds
  {
    Serial.printf("Button States - B1: %s (%s), B2: %s (%s), Dispensing: %s\n",
                  button1State ? "HIGH" : "LOW",
                  button1State ? "NOT PRESSED" : "PRESSED",
                  button2State ? "HIGH" : "LOW",
                  button2State ? "NOT PRESSED" : "PRESSED",
                  feederSystem.dispensing ? "YES" : "NO");
    lastDebugPrint = currentTime;
  }

  // Handle Button 1 (Manual Feed/Dispense) - ACTIVE LOW (pressed = LOW)
  // Only trigger on falling edge (HIGH to LOW transition)
  if (button1State == LOW && lastButton1State == HIGH &&
      (currentTime - lastButton1Press) > debounceDelay)
  {
    Serial.println("=== BUTTON 1 PRESSED - Manual feed triggered ===");
    lastButton1Press = currentTime;

    // Check if enough time has passed since last dispensing
    bool canDispense = !feederSystem.dispensing && 
                      (currentTime - timing.lastFeedingTime >= 1000); // Reduced from 5000ms to 1000ms
    
    if (canDispense)
    {
      // Call performManualFeed directly to override all restrictions
      Serial.println("Starting MANUAL OVERRIDE feeding sequence...");
      performManualFeed();
    }
    else
    {
      // Show a message explaining why we can't dispense
      if (feederSystem.dispensing) {
        Serial.println("Cannot dispense - already dispensing (button press ignored)");
      } else {
        Serial.println("Cannot dispense - cooldown period (button press ignored)");
        
        // Show cooldown message on LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Please wait");
        lcd.setCursor(0, 1);
        lcd.print("Cooldown active");
        delay(1000); // Show message briefly
      }
    }
  }

  // Handle Button 2 (Settings/Mode toggle) - ACTIVE LOW
  if (button2State == LOW && lastButton2State == HIGH &&
      (currentTime - lastButton2Press) > debounceDelay)
  {
    Serial.println("=== BUTTON 2 PRESSED - Toggle auto feeding ===");
    lastButton2Press = currentTime;

    // Toggle auto feeding mode
    feederSystem.autoFeedingEnabled = !feederSystem.autoFeedingEnabled;

    Serial.printf("Auto feeding is now: %s\n",
                  feederSystem.autoFeedingEnabled ? "ENABLED" : "DISABLED");

    // Show status on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Auto Feeding:");
    lcd.setCursor(0, 1);
    lcd.print(feederSystem.autoFeedingEnabled ? "ENABLED " : "DISABLED");
    delay(2000);
  }

  // Update last states
  lastButton1State = button1State;
  lastButton2State = button2State;
}

void performManualFeed()
{
  // Set dispensing flag immediately to prevent multiple triggers
  feederSystem.dispensing = true;
  timing.dispenseStartTime = millis();

  Serial.println("=== STARTING MANUAL FEED SEQUENCE ===");

  // Display dispensing status on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Manual Dispensing");

  // Servo dispensing: 90° to 45° and back, 3 times with 1s delay
  for (int i = 0; i < 3; i++)
  {
    Serial.printf("Dispensing cycle %d/3\n", i + 1);

    // Update LCD with cycle number
    lcd.setCursor(0, 1);
    lcd.print("Cycle ");
    lcd.print(i + 1);
    lcd.print(" of 3     ");

    // Move servo to 45 degrees (left)
    myServo.write(45);

    // Buzzer sound when moving
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);

    delay(500); // Wait in position

    // Return servo to 90 degrees (center)
    myServo.write(90);

    // Buzzer sound when returning
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);

    // 1 second delay between cycles (except after last cycle)
    if (i < 2)
    {
      delay(1000);
    }
  }

  // Final confirmation
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Complete!");
  lcd.setCursor(0, 1);
  lcd.print("Food Dispensed");

  // Final buzzer sequence
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    delay(150);
  }

  delay(2000); // Show completion message

  // Clear dispensing flag and update timing
  feederSystem.dispensing = false;
  timing.lastFeedingTime = millis();

  // Record the manual feeding
  recordFoodDispensing("Manual Override");

  Serial.println("=== MANUAL FEEDING SEQUENCE COMPLETED ===");
  Serial.println("Ready for next dispensing in 1 second");
}

bool isButton1Pressed()
{
  return digitalRead(BUTTON1_PIN) == LOW;
}

bool isButton2Pressed()
{
  return digitalRead(BUTTON2_PIN) == LOW;
}