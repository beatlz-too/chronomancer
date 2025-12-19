#include "display.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// Display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Reset pin not used

// I2C pins (from wiring diagram)
#define SDA_PIN 21  // D21
#define SCL_PIN 19  // D19

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Global state for title and body
static char displayTitle[32] = "";
static char displayBody[64] = "";

void initDisplay() {
  // Initialize I2C with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;); // Don't proceed, loop forever
  }
  
  // Clear the buffer
  display.clearDisplay();
  
  // Set default text properties
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  // Display initial message
  display.display();
  
  Serial.println("Display initialized");
}

// Internal function to update the entire display
void updateDisplay(const char* title, const char* body) {
  // Store the values
  strncpy(displayTitle, title, sizeof(displayTitle) - 1);
  displayTitle[sizeof(displayTitle) - 1] = '\0';
  
  strncpy(displayBody, body, sizeof(displayBody) - 1);
  displayBody[sizeof(displayBody) - 1] = '\0';
  
  // Clear display
  display.clearDisplay();
  
  // Draw title (larger text, bold)
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(title);
  
  // Draw separator line
  display.drawLine(0, 18, SCREEN_WIDTH, 18, SSD1306_WHITE);
  
  // Draw body (smaller text)
  display.setTextSize(1);
  display.setCursor(0, 22);
  display.println(body);
  
  // Update display
  display.display();
}

// Public function: Update title (keeps current body)
void updateDisplayTitle(const char* title) {
  updateDisplay(title, displayBody);
}

// Public function: Update body (keeps current title)
void updateDisplayBody(const char* text) {
  updateDisplay(displayTitle, text);
}

