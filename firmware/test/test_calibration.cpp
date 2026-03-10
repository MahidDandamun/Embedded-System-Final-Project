// ============================================================================
// Load Cell Calibration Test
//
// Upload this test to your ESP32 to find the correct CALIBRATION_FACTOR.
//
// Steps:
//   1. Upload: pio test -e esp32dev -f test_calibration
//      (or copy this file to src/main.cpp temporarily)
//   2. Open serial monitor at 115200 baud
//   3. Follow the on-screen prompts
//   4. Place a known weight (e.g. 100g) on the bowl
//   5. Note the calibration factor printed
//   6. Update CALIBRATION_FACTOR in include/config.h
// ============================================================================

#include <Arduino.h>
#include <HX711.h>

// Use the same pins as config.h
#define HX711_DOUT_PIN 5
#define HX711_SCK_PIN  23

HX711 scale;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("========================================");
    Serial.println("  LOAD CELL CALIBRATION UTILITY");
    Serial.println("========================================");
    Serial.println();

    scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);

    if (!scale.is_ready()) {
        Serial.println("ERROR: HX711 not found. Check wiring:");
        Serial.printf("  DOUT -> GPIO %d\n", HX711_DOUT_PIN);
        Serial.printf("  SCK  -> GPIO %d\n", HX711_SCK_PIN);
        while (true) delay(1000);
    }

    Serial.println("HX711 detected.");
    Serial.println();

    // Step 1: Tare
    Serial.println("STEP 1: Remove all weight from the scale.");
    Serial.println("        Press ENTER in serial monitor when ready...");
    while (!Serial.available()) delay(10);
    while (Serial.available()) Serial.read();

    Serial.println("Taring... (averaging 20 readings)");
    scale.set_scale();
    scale.tare(20);
    Serial.printf("Zero offset: %ld\n", scale.get_offset());
    Serial.println();

    // Step 2: Known weight
    Serial.println("STEP 2: Place a KNOWN weight on the scale.");
    Serial.println("        Type the weight in grams (e.g. 100) and press ENTER:");

    while (!Serial.available()) delay(10);
    float knownWeight = Serial.parseFloat();
    while (Serial.available()) Serial.read();

    if (knownWeight <= 0) {
        Serial.println("Invalid weight. Restart and try again.");
        while (true) delay(1000);
    }

    Serial.printf("Known weight: %.1f g\n", knownWeight);
    Serial.println("Reading scale... (averaging 20 readings)");
    delay(2000);

    float rawReading = scale.get_units(20);
    float factor = rawReading / knownWeight;

    Serial.println();
    Serial.println("========================================");
    Serial.println("  CALIBRATION RESULT");
    Serial.println("========================================");
    Serial.printf("  Raw reading:       %.2f\n", rawReading);
    Serial.printf("  Known weight:      %.1f g\n", knownWeight);
    Serial.printf("  CALIBRATION_FACTOR: %.0f\n", factor);
    Serial.println("========================================");
    Serial.println();
    Serial.println("Copy this value to include/config.h:");
    Serial.printf("  #define CALIBRATION_FACTOR  %.0f\n", factor);
    Serial.println();

    // Verify
    scale.set_scale(factor);
    Serial.println("VERIFICATION — current readings:");
}

void loop() {
    if (scale.is_ready()) {
        float weight = scale.get_units(5);
        Serial.printf("Weight: %.2f g\n", weight);
    }
    delay(1000);
}
