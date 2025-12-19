#ifndef LEVEL_H
#define LEVEL_H

#include <Arduino.h>

// Initialize the level potentiometer
void initLevel();

// Update level value (reads from ADC)
void updateLevel(int* value);

// Get current level value (0-4095 raw ADC, or converted to percentage)
int getLevel();

#endif

