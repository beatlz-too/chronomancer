#include "level.h"

#define LEVEL_PIN 34  // D34 - ADC1_CH6

static int currentLevel = 0;

void initLevel() {
  // ADC is already initialized by default on ESP32
  // Pin 34 is ADC1_CH6, which is valid for analogRead
  Serial.println("Level potentiometer initialized");
}

void updateLevel(int* value) {
  // Read ADC value (0-4095 on ESP32)
  currentLevel = analogRead(LEVEL_PIN);
  if (value != nullptr) {
    *value = currentLevel;
  }
}

int getLevel() {
  return currentLevel;
}

