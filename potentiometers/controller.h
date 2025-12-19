#ifndef POTENTIOMETER_CONTROLLER_H
#define POTENTIOMETER_CONTROLLER_H

#include <Arduino.h>

// Initialize the potentiometer controller
void initPotentiometerController();

// Update all potentiometers and display (call this in loop())
void updatePotentiometersAndDisplay();

#endif

