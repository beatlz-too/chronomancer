#include "feedback.h"

#define FEEDBACK_PIN 33  // D33 - ADC1_CH5

static int currentFeedback = 0;

void initFeedback() {
  // ADC is already initialized by default on ESP32
  // Pin 33 is ADC1_CH5, which is valid for analogRead
  Serial.println("Feedback potentiometer initialized");
}

void updateFeedback(int* value) {
  // Read ADC value (0-4095 on ESP32)
  currentFeedback = analogRead(FEEDBACK_PIN);
  if (value != nullptr) {
    *value = currentFeedback;
  }
}

int getFeedback() {
  return currentFeedback;
}

