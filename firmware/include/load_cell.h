#ifndef LOAD_CELL_H
#define LOAD_CELL_H

#include "config.h"
#include "globals.h"

void setupLoadCell();
float readLoadCell();
void tareLoadCell();
void calibrateLoadCell(float knownWeight);

#endif // LOAD_CELL_H
