#include "delay_time.h"

#define DELAY_TIME_PIN 32  // D32 - ADC1_CH4

static int currentDelayTime = 0;

void initDelayTime() {
  // ADC is already initialized by default on ESP32
  // Pin 32 is ADC1_CH4, which is valid for analogRead
  Serial.println("Delay Time potentiometer initialized");
}

void updateDelayTime(int* value) {
  // Read ADC value (0-4095 on ESP32)
  currentDelayTime = analogRead(DELAY_TIME_PIN);
  if (value != nullptr) {
    *value = currentDelayTime;
  }
}

int getDelayTime() {
  return currentDelayTime;
}

