#include "load_cell.h"

void setupLoadCell() {
    scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);

    Serial.print("Load cell init...");
    int retries = 0;
    while (!scale.is_ready() && retries < 10) {
        delay(100);
        retries++;
    }

    if (scale.is_ready()) {
        scale.set_scale(CALIBRATION_FACTOR);
        scale.tare();
        Serial.println(" OK");
    } else {
        Serial.println(" FAILED — check wiring");
    }
}

float readLoadCell() {
    if (scale.is_ready()) {
        float val = scale.get_units(SCALE_READINGS);
        return val < 0 ? 0 : val;
    }
    return -1.0;
}

void tareLoadCell() {
    if (scale.is_ready()) {
        scale.tare();
        Serial.println("Load cell tared");
    }
}

void calibrateLoadCell(float knownWeight) {
    if (!scale.is_ready() || knownWeight <= 0) {
        Serial.println("Calibration failed: scale not ready or invalid weight");
        return;
    }

    scale.set_scale();
    scale.tare();
    Serial.println("Place known weight on scale and wait...");
    delay(5000);

    float reading = scale.get_units(10);
    float factor = reading / knownWeight;

    scale.set_scale(factor);
    Serial.printf("Calibration complete. Factor: %.2f\n", factor);
    Serial.printf("Verify: %.2f g (expected %.2f g)\n",
                  scale.get_units(5), knownWeight);
}
