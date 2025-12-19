#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// Initialize the display
void initDisplay();

// Update the display title
void updateDisplayTitle(const char* title);

// Update the display body text
void updateDisplayBody(const char* text);

#endif

