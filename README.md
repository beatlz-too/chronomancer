# ESP32 Guitar delay pedal

## Description
This is a prototypical project that uses an ESP32 microcontroller to emulate a delay guitar pedal. 

## MVP
Plug a guitar into the final circuit and output a delayed signal using an audio jack. 

### Requirements
- Two delays
- Can be controlled to split signal in parallel
- Can change between delay types
- Can upload a script with self-made delay
- Can modulate delay time
- Can modulate feedback level
- Can modulate delay level

## Equipment
- ESP32 microcontroller
- PCM 1808 DAC
- PCM 5102 ADC
- OLED 0.96" Display (SDD1306 I2 with GND, VDD, SCK, SDA pins)
- Potentiometers 10k ohm (linear)
- Buttons

## User interface
We'll use the potentiometers to control delay time, feedback, and level.

We'll use the screen to show what we're changing whenever we turn the pots.

We'll hardcode the delay type in compile time (for now, we might upgrade to encoders to pick types later)

We'll use the USB-C input integrated into the microcontroller to plug it to the PC and upload new delay programs to the pedal (this is in a later stage)

## Code architecture
Each part of the pedal should be handled independently. We're going to use a combination of some of the SOLID design ideas (agnosticity, single responsibility, reusability) with the idea of "Locality of behavior" to prevent useles granularity. Put each module in an individual folder.

We can use C++ files to run the code for each part and use `.h` exports/imports to compile everything into a single `.ino` file that will be uploaded to the microcontroller.

### Display
We want an individual file that has these exported functions:

`void updateDisplayTitle(<str> title)` -> Updates the title state
`void updateDisplayBody(<str> text)` -> Updates the body state

The tricky part with these is to keep a global variable with both values, since you can't individually update strings in the display. When you want to upadte the title, you need to upload the whole content to the display. So, in reality, both functions actually are syntactic sugar for a fn `updateDisplay(<str> title, <str> text)`, where the user doesn't need to keep track of the state of both strings to update one.

**Other functionality of the display's API**
If you want to update things in the display, such as display size, you can still simply call the display functions in whatever file.

### Potentiometers
We want a file per potentiometer. This means one for the feedack, one for the level, one for the delay time.

The exported functions should be just one per pot:

`void updateLevel(<int> lvl)`
`void updateFeedback(<int> lvl)`
`void updateDelayTime(<int> lvl)`

### Delay
We want one file per delay type.
We want one file with the code to handle the delay in the controller. *This can be skipped for now, we can simply hardcode the delay time in compile time. We'll make a mapper later when we have an encoder to pick delay types.*

This means that we will upload the delay type on `setup()` and we'll be using `loop()` to update the pot values in runtime.

### Display-Potentiometer controller
We need a file that takes care of updating the values of the display whenever the user rotates a pot. 

What we want is that the default value of the display is

Title: Delay name (e.g. Tape Delay)
Body: Defaults to current delay level in ms on `setup()`. Whenever the user uses a pot, it will show the name of the input (level, feedback, time) and the value. E.g., user rotates the time potentiometer, it shows in realtime

**Tape Delay**
*Time*
150ms

Then rotates the feedback pot

**Tape Delay**
*Feedback*
75%

## Execution plan
0. Have a `delay-pedal.ino` file ready with the `setup()` and `loop()` functions. This is our compilation file where we'll import all of our C++ modules.`
1. Display code and implementation: make sure the display is working from the get-go. We should have the name of the delay in the title. The body will show the level of the last value changed by the user, defaulting to delay time. So, if the user 
2. Potentiometer code and implementation. This means coding the potentiometers individual files and the display-potentiometer-controller.
3. Delays. We'll make individual files for the delays, which will be first hardcoded in `setup()`. We'll start with the simplest: a digital delay that uses the three params (time, feedback, level) from the pots.

## Wiring

### Power Distribution
- ESP32 3V3 → All component power pins (via distribution)
- ESP32 GND → All component GND pins (common ground)
- Add 100µF electrolytic capacitor: **positive (+) to ESP32 3V3**, negative (-) to ESP32 GND (power supply decoupling)
- Add 0.1µF (100nF) ceramic capacitor between 3V3 and GND near each IC (local decoupling - non-polarized, either direction)

### PCM 1808 ADC (Audio Input)
- [ ] PCM 1808 3.3V → ESP32 3V3
- [ ] PCM 1808 GND → ESP32 GND
- [ ] PCM 1808 SCK → ESP32 D25 (I2S Master Clock - GPIO 25)
- [ ] PCM 1808 BCK → ESP32 D4 (I2S Bit Clock - GPIO 4)
- [ ] PCM 1808 LRC → ESP32 D15 (I2S Left/Right Clock - GPIO 15)
- [ ] PCM 1808 OUT → ESP32 D27 (I2S Data Input - GPIO 27)
- [ ] PCM 1808 FMY → GND (I2S format, 0 = I2S mode)
- [ ] PCM 1808 MDO → GND (Mode select)
- [ ] PCM 1808 MDI → GND (Mode select)
- [ ] Add 0.1µF (100nF) capacitor between PCM 1808 3.3V and GND (local decoupling - non-polarized, either direction)
- [ ] Add 47µF capacitor: **positive (+) to PCM 1808 3.3V**, negative (-) to PCM 1808 GND (filtering/decoupling)

### PCM 5102 DAC (Audio Output)
- [ ] PCM 5102 VIN → ESP32 3V3
- [ ] PCM 5102 GND → ESP32 GND
- [ ] PCM 5102 SCK → ESP32 D25 (I2S Master Clock - shared with ADC, GPIO 25)
- [ ] PCM 5102 BCK → ESP32 D4 (I2S Bit Clock - shared with ADC, GPIO 4)
- [ ] PCM 5102 DIN → ESP32 D22 (I2S Data Output - GPIO 22)
- [ ] PCM 5102 LCK → ESP32 D15 (I2S Left/Right Clock - shared with ADC, GPIO 15)
- [ ] Add 0.1µF (100nF) capacitor between PCM 5102 VIN and GND (local decoupling - non-polarized, either direction)
- [ ] Add 47µF capacitor: **positive (+) to PCM 5102 VIN**, negative (-) to PCM 5102 GND (filtering/decoupling)

### SSD1306 OLED Display (I2C)
- [ ] Display VDD → ESP32 3V3
- [ ] Display GND → ESP32 GND
- [ ] Display SDA → ESP32 D21 (I2C Data - GPIO 21)
- [ ] Display SCK → ESP32 D19 (I2C Clock - GPIO 19)
- [ ] Add 4.7kΩ pull-up resistors on SDA and SCK lines (to 3V3)

### Potentiometers (3x 10kΩ linear)
**Delay Time Pot:**
- [ ] Pot 1 terminal 1 → ESP32 3V3
- [ ] Pot 1 terminal 2 (wiper) → ESP32 D32 (ADC1_CH4 - GPIO 32)
- [ ] Pot 1 terminal 3 → ESP32 GND

**Feedback Pot:**
- [ ] Pot 2 terminal 1 → ESP32 3V3
- [ ] Pot 2 terminal 2 (wiper) → ESP32 D33 (ADC1_CH5 - GPIO 33)
- [ ] Pot 2 terminal 3 → ESP32 GND

**Level Pot:**
- [ ] Pot 3 terminal 1 → ESP32 3V3
- [ ] Pot 3 terminal 2 (wiper) → ESP32 D34 (ADC1_CH6 - GPIO 34)
- [ ] Pot 3 terminal 3 → ESP32 GND

### Audio Input Jack (Guitar Input)
- [ ] Audio jack tip → 47µF coupling capacitor (**positive (+) toward jack tip**, negative (-) toward PCM 1808 LIN) → 10kΩ resistor → PCM 1808 LIN (Left Input)
- [ ] Audio jack ring/sleeve → ESP32 GND
- [ ] Add 1kΩ resistor from PCM 1808 LIN to GND (input bias/impedance)
- [ ] PCM 1808 RIN → GND (Right Input tied to ground for mono/single-ended input)

### Additional Components Needed
- [ ] 1x 100µF electrolytic capacitor (power supply decoupling) - **You have this**
- [ ] 3x 47µF capacitors (filtering/decoupling: PCM 1808, PCM 5102, and audio input coupling) - **You have these**
- [ ] 2-3x 0.1µF (100nF) ceramic capacitors (local IC decoupling: PCM 1808, PCM 5102, and power distribution) - **You have these**
- [ ] 2x 4.7kΩ resistors (I2C pull-ups)
- [ ] 1x 10kΩ resistor (audio input bias)
- [ ] 1x 1kΩ resistor (audio input impedance)
- [ ] 3x 10kΩ linear potentiometers (already listed)

### Pin Summary
**ESP32 Pin Assignments:**
- D4 (GPIO 4): I2S BCLK (Bit Clock) - shared ADC/DAC
- D15 (GPIO 15): I2S LRCLK (Left/Right Clock) - shared ADC/DAC
- D19 (GPIO 19): I2C SCK (Display clock)
- D21 (GPIO 21): I2C SDA (Display data)
- D22 (GPIO 22): I2S DIN (to PCM 5102)
- D25 (GPIO 25): I2S MCLK (Master Clock) - shared ADC/DAC
- D27 (GPIO 27): I2S Data Input (from PCM 1808)
- D32 (GPIO 32): ADC - Delay Time Pot
- D33 (GPIO 33): ADC - Feedback Pot
- D34 (GPIO 34): ADC - Level Pot

**Important Notes:**
1. Audio ground: Ensure all audio components share a common ground with the ESP32.
2. Power: ESP32 can be powered via USB-C or external 5V supply via VIN pin (regulated to 3.3V internally).
3. I2S sharing: ADC and DAC can share MCLK (D25), BCLK (D4), and LRCLK (D15) since they're on the same I2S bus.
4. ADC pins: D32, D33, D34 are ADC1 channels and work well for potentiometers.
5. I2C pins: D21 (SDA) and D19 (SCL) are standard I2C pins for the display.