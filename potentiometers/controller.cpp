#include "controller.h"
#include "delay_time.h"
#include "feedback.h"
#include "level.h"
#include "display/display.h"

// Threshold for detecting pot changes (to avoid constant updates)
// Lower value = more sensitive to changes, higher value = less flickering
#define POT_CHANGE_THRESHOLD 30  // ADC units (increased to filter noise better)

// Threshold for updating display value (to avoid flickering from small fluctuations)
// Higher value = less flickering, but less responsive
#define DISPLAY_UPDATE_THRESHOLD 15  // ADC units - only update display if value changed this much

// Number of samples to average for smoothing
#define FILTER_SAMPLES 10  // Increased for better smoothing

// Last read values (filtered/averaged)
static int lastDelayTime = -1;
static int lastFeedback = -1;
static int lastLevel = -1;

// Last displayed values (to prevent unnecessary display updates)
static int lastDisplayedDelayTime = -1;
static int lastDisplayedFeedback = -1;
static int lastDisplayedLevel = -1;

// Filter buffers for averaging
static int delayTimeBuffer[FILTER_SAMPLES];
static int feedbackBuffer[FILTER_SAMPLES];
static int levelBuffer[FILTER_SAMPLES];
static int delayTimeIndex = 0;
static int feedbackIndex = 0;
static int levelIndex = 0;
static bool filterInitialized = false;

// Simple moving average filter
int filterValue(int* buffer, int newValue, int* index) {
  // Initialize buffer with first reading if not initialized
  if (!filterInitialized) {
    for (int i = 0; i < FILTER_SAMPLES; i++) {
      buffer[i] = newValue;
    }
  }
  
  buffer[*index] = newValue;
  *index = (*index + 1) % FILTER_SAMPLES;
  
  long sum = 0;
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    sum += buffer[i];
  }
  return sum / FILTER_SAMPLES;
}

// Track which pot was last changed (for display)
static enum {
  POT_NONE,
  POT_DELAY_TIME,
  POT_FEEDBACK,
  POT_LEVEL
} lastChangedPot = POT_NONE;

// Convert ADC value (0-4095) to delay time in milliseconds
// Assuming pot range maps to 0-500ms delay
int adcToDelayTime(int adcValue) {
  // Map 0-4095 to 0-500ms
  return map(adcValue, 0, 4095, 0, 500);
}

// Convert ADC value (0-4095) to percentage (0-100)
int adcToPercentage(int adcValue) {
  return map(adcValue, 0, 4095, 0, 100);
}

void initPotentiometerController() {
  // Initialize all potentiometers
  initDelayTime();
  initFeedback();
  initLevel();
  
  // Read initial values multiple times to fill filter buffers
  int tempDelay, tempFeedback, tempLevel;
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    updateDelayTime(&tempDelay);
    updateFeedback(&tempFeedback);
    updateLevel(&tempLevel);
    filterValue(delayTimeBuffer, tempDelay, &delayTimeIndex);
    filterValue(feedbackBuffer, tempFeedback, &feedbackIndex);
    filterValue(levelBuffer, tempLevel, &levelIndex);
    delay(10); // Small delay between readings
  }
  filterInitialized = true;
  
  // Get filtered initial values
  lastDelayTime = filterValue(delayTimeBuffer, tempDelay, &delayTimeIndex);
  lastFeedback = filterValue(feedbackBuffer, tempFeedback, &feedbackIndex);
  lastLevel = filterValue(levelBuffer, tempLevel, &levelIndex);
  
  // Set initial display to delay time (as per README)
  char buffer[32];
  int delayMs = adcToDelayTime(lastDelayTime);
  snprintf(buffer, sizeof(buffer), "Time: %dms", delayMs);
  updateDisplayBody(buffer);
  
  lastChangedPot = POT_DELAY_TIME;
  
  Serial.println("Potentiometer controller initialized");
}

void updatePotentiometersAndDisplay() {
  static unsigned long lastUpdateTime = 0;
  const unsigned long UPDATE_INTERVAL = 50; // Update display every 50ms to avoid flicker
  
  int currentDelayTime, currentFeedback, currentLevel;
  int delayTimeChange = 0;
  int feedbackChange = 0;
  int levelChange = 0;
  char buffer[64];
  
  // Read all potentiometers (raw values)
  int rawDelayTime, rawFeedback, rawLevel;
  updateDelayTime(&rawDelayTime);
  updateFeedback(&rawFeedback);
  updateLevel(&rawLevel);
  
  // Apply filtering/averaging to smooth out noise
  currentDelayTime = filterValue(delayTimeBuffer, rawDelayTime, &delayTimeIndex);
  currentFeedback = filterValue(feedbackBuffer, rawFeedback, &feedbackIndex);
  currentLevel = filterValue(levelBuffer, rawLevel, &levelIndex);
  
  // Calculate changes for all pots (before updating stored values)
  delayTimeChange = abs(currentDelayTime - lastDelayTime);
  feedbackChange = abs(currentFeedback - lastFeedback);
  levelChange = abs(currentLevel - lastLevel);
  
  // Determine which pot changed the most (even if small)
  // Use a lower threshold for detecting any change, but still require significant change to switch
  int maxChange = 0;
  int potToShow = POT_NONE;
  const int MIN_CHANGE_THRESHOLD = 3; // Lower threshold to detect any pot movement
  
  // Find which pot changed the most (even if below main threshold)
  if (delayTimeChange > maxChange) {
    maxChange = delayTimeChange;
    potToShow = POT_DELAY_TIME;
  }
  
  if (feedbackChange > maxChange) {
    maxChange = feedbackChange;
    potToShow = POT_FEEDBACK;
  }
  
  if (levelChange > maxChange) {
    maxChange = levelChange;
    potToShow = POT_LEVEL;
  }
  
  // Only switch if:
  // 1. The pot changed above minimum threshold (to avoid noise)
  // 2. AND either:
  //    - It's a new pot (lastChangedPot is NONE)
  //    - OR the new pot changed significantly more than the current pot
  if (potToShow != POT_NONE && maxChange >= MIN_CHANGE_THRESHOLD) {
    int currentPotChange = 0;
    switch (lastChangedPot) {
      case POT_DELAY_TIME: currentPotChange = delayTimeChange; break;
      case POT_FEEDBACK: currentPotChange = feedbackChange; break;
      case POT_LEVEL: currentPotChange = levelChange; break;
      default: currentPotChange = 0; break;
    }
    
    // Switch if:
    // - No pot is currently selected (POT_NONE)
    // - OR the new pot is different AND changed significantly more (3x) than current pot
    //   This creates hysteresis - once a pot is selected, it's "sticky" and requires
    //   a much larger change from another pot to switch away
    if (lastChangedPot == POT_NONE) {
      // First time - just select the pot that changed
      lastChangedPot = (decltype(lastChangedPot))potToShow;
    } else if (potToShow != lastChangedPot) {
      // Different pot - only switch if it changed at least 3x more than current pot
      // This prevents rapid switching due to small fluctuations
      if (maxChange >= (currentPotChange * 3)) {
        lastChangedPot = (decltype(lastChangedPot))potToShow;
      }
    }
  }
  
  // Debug output (every second)
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 1000) {
    Serial.printf("Raw ADC - Delay: %d, Feedback: %d, Level: %d\n", 
                  rawDelayTime, rawFeedback, rawLevel);
    Serial.printf("Filtered ADC - Delay: %d, Feedback: %d, Level: %d\n", 
                  currentDelayTime, currentFeedback, currentLevel);
    Serial.printf("Converted - Delay: %dms, Feedback: %d%%, Level: %d%%\n",
                  adcToDelayTime(currentDelayTime), 
                  adcToPercentage(currentFeedback),
                  adcToPercentage(currentLevel));
    Serial.printf("Changes - Delay: %d, Feedback: %d, Level: %d\n", 
                  delayTimeChange, feedbackChange, levelChange);
    Serial.printf("Showing pot: %d (Delay=%d, Feedback=%d, Level=%d)\n",
                  lastChangedPot, POT_DELAY_TIME, POT_FEEDBACK, POT_LEVEL);
    lastDebugTime = millis();
  }
  
  // Update display periodically (to avoid constant redraws) or when a pot changes significantly
  unsigned long currentTime = millis();
  bool valueChangedSignificantly = false;
  bool shouldUpdateTime = false;
  
  // Check if the currently displayed pot's value changed significantly
  // Use converted values for comparison to match what's displayed
  // Use larger thresholds to prevent flickering from noise
  switch (lastChangedPot) {
    case POT_DELAY_TIME: {
      int currentMs = adcToDelayTime(currentDelayTime);
      int lastMs = (lastDisplayedDelayTime == -1) ? -1 : adcToDelayTime(lastDisplayedDelayTime);
      // Compare in milliseconds - update if changed by at least 5ms (to filter out 1-3ms noise)
      if (lastDisplayedDelayTime == -1 || abs(currentMs - lastMs) >= 5) {
        valueChangedSignificantly = true;
        shouldUpdateTime = true;
      }
      break;
    }
    case POT_FEEDBACK: {
      int currentPercent = adcToPercentage(currentFeedback);
      int lastPercent = (lastDisplayedFeedback == -1) ? -1 : adcToPercentage(lastDisplayedFeedback);
      // Compare in percentage - update if changed by at least 2% (to filter out 1% noise)
      if (lastDisplayedFeedback == -1 || abs(currentPercent - lastPercent) >= 2) {
        valueChangedSignificantly = true;
      }
      break;
    }
    case POT_LEVEL: {
      int currentPercent = adcToPercentage(currentLevel);
      int lastPercent = (lastDisplayedLevel == -1) ? -1 : adcToPercentage(lastDisplayedLevel);
      // Compare in percentage - update if changed by at least 2% (to filter out 1% noise)
      if (lastDisplayedLevel == -1 || abs(currentPercent - lastPercent) >= 2) {
        valueChangedSignificantly = true;
      }
      break;
    }
    default:
      shouldUpdateTime = true; // First time, always update
      break;
  }
  
  bool shouldUpdate = valueChangedSignificantly || 
                      (potToShow != POT_NONE && potToShow != lastChangedPot) ||
                      (currentTime - lastUpdateTime > UPDATE_INTERVAL);
  
  if (shouldUpdate || shouldUpdateTime) {
    // Update display with current value of the last changed pot
    switch (lastChangedPot) {
      case POT_DELAY_TIME: {
        int delayMs = adcToDelayTime(currentDelayTime);
        snprintf(buffer, sizeof(buffer), "Time\n%dms", delayMs);
        updateDisplayBody(buffer);
        lastDisplayedDelayTime = currentDelayTime;
        break;
      }
      case POT_FEEDBACK: {
        int feedbackPercent = adcToPercentage(currentFeedback);
        snprintf(buffer, sizeof(buffer), "Feedback\n%d%%", feedbackPercent);
        updateDisplayBody(buffer);
        lastDisplayedFeedback = currentFeedback;
        break;
      }
      case POT_LEVEL: {
        int levelPercent = adcToPercentage(currentLevel);
        snprintf(buffer, sizeof(buffer), "Level\n%d%%", levelPercent);
        updateDisplayBody(buffer);
        lastDisplayedLevel = currentLevel;
        break;
      }
      default:
        // If no pot has been changed yet, default to delay time
        if (lastChangedPot == POT_NONE) {
          int delayMs = adcToDelayTime(currentDelayTime);
          snprintf(buffer, sizeof(buffer), "Time\n%dms", delayMs);
          updateDisplayBody(buffer);
          lastChangedPot = POT_DELAY_TIME;
          lastDisplayedDelayTime = currentDelayTime;
        }
        break;
    }
    lastUpdateTime = currentTime;
  }
  
  // Update stored values (after checking for changes)
  lastDelayTime = currentDelayTime;
  lastFeedback = currentFeedback;
  lastLevel = currentLevel;
}

