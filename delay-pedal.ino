/*
 * ESP32 Guitar Delay Pedal
 * Main compilation file - imports all C++ modules
 */

#include "display/display.h"
#include "display/display.cpp"

// Potentiometer modules
#include "potentiometers/delay_time.h"
#include "potentiometers/delay_time.cpp"
#include "potentiometers/feedback.h"
#include "potentiometers/feedback.cpp"
#include "potentiometers/level.h"
#include "potentiometers/level.cpp"
#include "potentiometers/controller.h"
#include "potentiometers/controller.cpp"

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("Delay Pedal Starting...");
  
  // Initialize display module
  initDisplay();
  updateDisplayTitle("Tape Delay");
  
  // Initialize potentiometer controller (this also initializes all pots and sets initial display)
  initPotentiometerController();
  
  // TODO: Initialize delay module
  // TODO: Initialize I2S audio (ADC and DAC)
  
  Serial.println("Setup complete!");
}

void loop() {
  // Update potentiometers and display (reads pots and updates display automatically)
  updatePotentiometersAndDisplay();
  
  // TODO: Update delay parameters based on potentiometers
  // TODO: Process audio through delay
}

