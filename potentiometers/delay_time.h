#ifndef DELAY_TIME_H
#define DELAY_TIME_H

#include <Arduino.h>

// Initialize the delay time potentiometer
void initDelayTime();

// Update delay time value (reads from ADC)
void updateDelayTime(int* value);

// Get current delay time value (0-4095 raw ADC, or converted to ms)
int getDelayTime();

#endif

