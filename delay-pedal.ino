/*
 * ESP32 Guitar Delay Pedal
 * Main compilation file - imports all C++ modules
 */

#include "display/display.h"
#include "display/display.cpp"

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("Delay Pedal Starting...");
  
  // Initialize display module
  initDisplay();
  updateDisplayTitle("Tape Delay");
  updateDisplayBody("Time: 150ms");
  
  // TODO: Initialize potentiometer modules
  // TODO: Initialize delay module
  // TODO: Initialize I2S audio (ADC and DAC)
  
  Serial.println("Setup complete!");
}

void loop() {
  // TODO: Read potentiometer values
  // TODO: Update delay parameters based on potentiometers
  // TODO: Process audio through delay
  // TODO: Update display with current values
}

