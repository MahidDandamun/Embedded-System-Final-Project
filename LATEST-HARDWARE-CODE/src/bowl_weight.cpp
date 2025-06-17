#include "bowl_weight.h"
#include "system_data.h" // Include to update feederSystem structure

HX711 scale;
bool loadCellReady = false;

void setupLoadCell()
{
  // Initialize HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  if (scale.is_ready())
  {
    scale.set_scale(CALIBRATION_FACTOR);
    scale.tare(); // Reset scale to zero
    loadCellReady = true;

    // Update system status
    feederSystem.loadCellReady = true;
  }
  else
  {
    loadCellReady = false;
    feederSystem.loadCellReady = false;
  }
}

float getBowlWeight()
{
  if (!loadCellReady || !scale.is_ready())
  {
    return -1; // Error value
  }

  // Get weight in grams (averaged over multiple readings)
  float weight = scale.get_units(WEIGHT_READINGS);

  // Return absolute value in case of negative readings when empty
  return weight < 0 ? 0.0 : weight;
}

String getBowlWeightStatus()
{
  float weight = getBowlWeight();

  if (weight < 0)
  {
    return "Error";
  }
  else if (weight <= BOWL_EMPTY_THRESHOLD)
  {
    return "Empty";
  }
  else if (weight <= BOWL_LOW_THRESHOLD)
  {
    return "Low";
  }
  else if (weight <= BOWL_MEDIUM_THRESHOLD)
  {
    return "Medium";
  }
  else
  {
    return "Full";
  }
}

void tareLoadCell()
{
  if (loadCellReady)
  {
    scale.tare();
  }
}
