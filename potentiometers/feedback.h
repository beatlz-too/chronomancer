#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <Arduino.h>

// Initialize the feedback potentiometer
void initFeedback();

// Update feedback value (reads from ADC)
void updateFeedback(int* value);

// Get current feedback value (0-4095 raw ADC, or converted to percentage)
int getFeedback();

#endif

