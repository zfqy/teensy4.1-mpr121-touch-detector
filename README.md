# Teensy 4.1 + MPR121 Touch Detection Project

This project uses a Teensy 4.1 to read touch states from an MPR121 capacitive touch sensor, and outputs the touch results of MPR121 channels 0 and 1 on Teensy digital pins. The current example enables 2 channels by default, but the code structure can be extended to support all 12 MPR121 touch channels.

Current sketch file:

```text
mpr121_teensy_touch/mpr121_teensy_touch.ino
```

## Features

- Reads the MPR121 over I2C.
- Detects the MPR121 E0 and E1 touch channels.
- Enables 2 channels by default, but can be extended to 12 channels by changing the configuration and output mapping.
- Prints touch and release events over Serial.
- Outputs the E0 detection result on Teensy digital PIN1.
- Outputs the E1 detection result on Teensy digital PIN2.
- Outputs HIGH when touched, and LOW when not touched.
- Raw debug output can be enabled with a switch.
- Touched mask summary output can be enabled with a switch.
- Uses software baseline detection, which makes it easier to observe and tune behavior under different electrode conditions.

## Hardware Wiring

Connect the MPR121 to the Teensy 4.1 as follows:

| MPR121 | Teensy 4.1 |
| --- | --- |
| SDA | PIN 18 / SDA0 |
| SCL | PIN 19 / SCL0 |
| IRQ | Digital PIN 0 |
| ADDR | GND |
| GND | GND |
| VIN / VCC | 3.3V, or the voltage supported by your module |

When ADDR is connected to GND, the MPR121 I2C address is:

```cpp
0x5A
```

Output pins:

| MPR121 Channel | Teensy Output Pin | Not Touched | Touched |
| --- | --- | --- | --- |
| E0 | PIN 1 | LOW / 0V | HIGH / 3.3V |
| E1 | PIN 2 | LOW / 0V | HIGH / 3.3V |

Note: Teensy 4.1 uses 3.3V logic. Do not connect PIN1/PIN2 directly to a device that may feed 5V back into the Teensy.

## Software Setup

Install the following:

- Arduino IDE 2.x
- Teensy by PJRC board support package
- Adafruit MPR121 library
- Adafruit BusIO library

Install Teensy support in Arduino IDE:

1. Open `File > Preferences`
2. Add the following URL to `Additional boards manager URLs`:

```text
https://www.pjrc.com/teensy/package_teensy_index.json
```

3. Open `Tools > Board > Boards Manager`
4. Search for and install `Teensy by PJRC`

Install the required libraries:

1. Open `Tools > Manage Libraries`
2. Search for and install `Adafruit MPR121`
3. If prompted, install the `Adafruit BusIO` dependency

## Usage

1. Open the sketch in Arduino IDE:

```text
mpr121_teensy_touch/mpr121_teensy_touch.ino
```

2. Select the board:

```text
Tools > Board > Teensy > Teensy 4.1
Tools > USB Type > Serial
```

3. Compile and upload.

When uploading to a Teensy for the first time, if Arduino IDE cannot find the board, press the white button on the Teensy 4.1 during the upload stage.

4. Open the Serial Monitor:

```text
Tools > Serial Monitor
```

Set the baud rate to:

```text
115200
```

On a normal startup, you should see something like:

```text
MPR121 multi-channel touch test starting...
Calibrating software baseline. Do not touch electrodes...
Software baseline: 0=... 1=...
Ready. Touch one or more electrodes.
```

Do not touch the electrodes during startup, because the program calibrates the software baseline first.

## Serial Output

When E0 is touched normally:

```text
Electrode 0 touched
Electrode 0 released
```

When E1 is touched normally:

```text
Electrode 1 touched
Electrode 1 released
```

If touched mask output is enabled, you will also see output like:

```text
Touched mask: 0b01 | channels: 0
```

Touched mask output is disabled by default.

## Important Configuration

There are several commonly used configuration values near the top of the code:

```cpp
constexpr uint8_t CHANNEL_0_OUTPUT_PIN = 1;
constexpr uint8_t CHANNEL_1_OUTPUT_PIN = 2;
constexpr uint8_t ELECTRODE_COUNT = 2;

constexpr uint8_t TOUCH_THRESHOLD = 5;
constexpr uint8_t RELEASE_THRESHOLD = 3;

constexpr bool ENABLE_DEBUG_OUTPUT = false;
constexpr bool ENABLE_TOUCH_MASK_OUTPUT = false;
```

Meaning:

- `CHANNEL_0_OUTPUT_PIN`: outputs the E0 touch result on Teensy PIN1.
- `CHANNEL_1_OUTPUT_PIN`: outputs the E1 touch result on Teensy PIN2.
- `ELECTRODE_COUNT`: currently set to 2 because only E0 and E1 are detected.
- `TOUCH_THRESHOLD`: threshold for detecting touch.
- `RELEASE_THRESHOLD`: threshold for detecting release.
- `ENABLE_DEBUG_OUTPUT`: whether to print Raw debug data.
- `ENABLE_TOUCH_MASK_OUTPUT`: whether to print the touched mask summary.

## Extending to 12 Channels

The MPR121 supports up to 12 touch channels, from E0 to E11. The current code only enables E0 and E1 to match the actual needs of this project:

```cpp
constexpr uint8_t ELECTRODE_COUNT = 2;
```

To detect all 12 channels, change it to:

```cpp
constexpr uint8_t ELECTRODE_COUNT = 12;
```

After this change, `calibrateSoftwareBaseline()`, `readSoftwareTouched()`, Raw debug output, and touched mask output will loop over all 12 channels.

Note that only E0 and E1 are currently mapped to Teensy digital output pins:

```cpp
constexpr uint8_t CHANNEL_0_OUTPUT_PIN = 1;
constexpr uint8_t CHANNEL_1_OUTPUT_PIN = 2;
```

If you want E2 through E11 to also drive Teensy digital output pins, add more output pin configuration values and extend the `updateTouchOutputPins()` function.

Also, different channels may have different electrode areas, wire lengths, and mounting conditions. After extending to 12 channels, per-channel thresholds are recommended instead of using one shared `TOUCH_THRESHOLD` for every channel.

## Debugging

If touch is not detected, first enable Raw debug output:

```cpp
constexpr bool ENABLE_DEBUG_OUTPUT = true;
```

After uploading again, the Serial Monitor will periodically print:

```text
Raw: 0:filtered/baseline/delta  1:filtered/baseline/delta
```

Example:

```text
Raw: 0:1/11/10  1:10/11/1
```

The three values mean:

- `filtered`: the current filtered real-time reading.
- `baseline`: the idle baseline calibrated at startup.
- `delta`: `baseline - filtered`, which is the current amount of change.

When touched, `filtered` usually decreases and `delta` increases.

The current detection logic is:

```cpp
delta >= TOUCH_THRESHOLD
```

If `delta` clearly increases when touched but the event is not triggered, lower `TOUCH_THRESHOLD`.

If false touches happen when nothing is touched, increase `TOUCH_THRESHOLD`, or improve the electrode and wiring environment.

## Threshold Notes

Different scenarios may require different thresholds. This is common for capacitive touch sensors. Influencing factors include:

- Electrode area.
- Wire length.
- Whether wires are close to metal, USB cables, or power cables.
- Coupling between the user's body and circuit ground.
- Desk or mounting material.
- Ambient humidity.
- Whether unused channels are floating.

This project currently uses fixed thresholds:

```cpp
TOUCH_THRESHOLD = 5
RELEASE_THRESHOLD = 3
```

These values work for the current test setup, but may need to be adjusted after changing electrodes, mounting position, or wire length.

For a more stable product-like version, possible future improvements include:

- Per-channel thresholds.
- Automatically measuring environmental noise at startup and estimating thresholds.
- Slowly updating the baseline when no touch is detected for a long time.
- Requiring multiple consecutive confirmations to avoid short noise spikes.
- Disabling unused channels, or avoiding floating unused electrodes.

## Common Issues

### The Teensy onboard LED stops blinking after upload

This is normal. The factory default Teensy program usually blinks the onboard LED. After uploading this project, the default blink program is replaced.

In the current code, the onboard LED is used as a heartbeat and toggles every 500 ms to confirm that the program is still running.

### Arduino IDE cannot find Teensy Loader

If you see an error like this while compiling or exporting a hex file:

```text
Opening Teensy Loader...
Unable find Teensy Loader.  (p)  Is the Teensy Loader application running?
Is a firewall (eg, ZoneAlarm) blocking localhost communication?
quitexit status 1
```

Check the following first:

- Whether Teensy Loader is already open.
- Whether `Operation > Auto` is checked in Teensy Loader.
- Whether `Teensy by PJRC` is installed in Arduino IDE.
- Whether the firewall allows `teensy.exe`, `teensy_post_compile.exe`, and `arduino-ide.exe`.

During actual debugging for this project, Teensy Loader was already open, but Arduino IDE still repeatedly reported that it could not find Teensy Loader. Restarting the computer fixed the issue.

So if the installation and settings look correct but the error keeps happening, restart the computer and try again.

### Serial shows `MPR121 not found at 0x5A`

This means the program is running, but it cannot find the MPR121 over I2C. Check:

- Whether SDA is connected to PIN18.
- Whether SCL is connected to PIN19.
- Whether ADDR is connected to GND.
- Whether Teensy and MPR121 share GND.
- Whether the MPR121 power supply is correct.
- Whether SDA/SCL are swapped.

### Serial Monitor cannot connect or shows no output

If the program was uploaded successfully but Serial Monitor cannot connect, or no startup message appears, first check the port selection in Arduino IDE:

```text
Tools > Port
```

Select the port that corresponds to the Teensy. On Windows it may appear as:

```text
COMx (Teensy 4.1)
```

It may also appear as something like:

```text
usb:0/140000/0/A/1
```

If the wrong COM port is selected, Serial Monitor will not connect to the Teensy and you will not see program output.

Recommended Serial Monitor baud rate:

```text
115200
```

### Touch does not respond

First enable:

```cpp
ENABLE_DEBUG_OUTPUT = true
```

Observe whether the `delta` value of the corresponding channel increases when touched.

If `delta` does not change at all, the electrode is usually not connected to the expected channel, or the touch area is too small.

If `delta` increases but no touch event is triggered, adjust the threshold.

### False touch events happen when nothing is touched

Possible causes:

- Threshold is too low.
- Electrode wires are too long.
- Electrodes are close to metal or power cables.
- Your hand is too close during startup calibration.
- Unused channels are floating.

Try power-cycling first, and do not touch the electrodes during calibration.

## Current Output Logic

The core output function in the code is:

```cpp
void updateTouchOutputPins(uint16_t touched) {
  digitalWrite(CHANNEL_0_OUTPUT_PIN, (touched & (1 << 0)) ? HIGH : LOW);
  digitalWrite(CHANNEL_1_OUTPUT_PIN, (touched & (1 << 1)) ? HIGH : LOW);
}
```

Therefore:

- When E0 is touched, PIN1 is HIGH.
- When E0 is released, PIN1 is LOW.
- When E1 is touched, PIN2 is HIGH.
- When E1 is released, PIN2 is LOW.

## Program Structure and Arduino C++ Notes

The control program in this project is written in Arduino-style C++. The `.ino` file looks similar to C, but it is compiled by the Arduino/Teensy toolchain using a C++ compiler.

One important feature of Arduino C++ is that you usually do not need to write your own `main()` function. You only need to implement:

```cpp
void setup() {
  // Runs once after power-on or reset
}

void loop() {
  // Runs repeatedly after setup()
}
```

Behind the scenes, the Arduino/Teensy framework does something roughly like this:

```cpp
int main() {
  init();
  setup();

  while (true) {
    loop();
  }
}
```

So `loop()` does not keep itself from exiting forever. Instead, the outer framework repeatedly calls it. Once one `loop()` execution finishes, it returns to the Arduino framework, and the framework immediately calls `loop()` again.

This project is roughly organized into the following parts:

### 1. Library imports

```cpp
#include <Wire.h>
#include <Adafruit_MPR121.h>
```

Where:

- `Wire.h` is used for I2C communication.
- `Adafruit_MPR121.h` is used to control the MPR121 touch sensor.

### 2. Global constants, variables, and object instantiation

Example:

```cpp
constexpr uint8_t MPR121_ADDR = 0x5A;
constexpr uint8_t CHANNEL_0_OUTPUT_PIN = 1;
constexpr uint8_t TOUCH_THRESHOLD = 5;

Adafruit_MPR121 cap = Adafruit_MPR121();
```

These define:

- The MPR121 I2C address.
- The input/output pins used by the Teensy.
- Touch and release thresholds.
- Debug switches.
- The MPR121 control object `cap`.

### 3. Helper function implementations

The program splits specific tasks into helper functions, such as:

```cpp
calibrateSoftwareBaseline();
readSoftwareTouched();
updateTouchOutputPins();
printTouchedChannels();
```

This keeps `setup()` and `loop()` easier to read.

Main responsibilities:

- `calibrateSoftwareBaseline()`: samples the no-touch state at startup and builds the software baseline.
- `readSoftwareTouched()`: reads current sensor values and determines which channels are touched.
- `updateTouchOutputPins()`: updates Teensy PIN1/PIN2 based on the E0/E1 touch states.
- `printTouchedChannels()`: optionally prints the touched mask summary.

### 4. `setup()` implementation

`setup()` runs once after power-on, reset, or upload.

In this project, `setup()` mainly:

- Starts USB Serial.
- Configures IRQ, output pins, and the onboard LED.
- Starts I2C.
- Initializes the MPR121.
- Sets touch thresholds.
- Calibrates the software baseline.
- Initializes output pin states.
- Prints startup information.

### 5. `loop()` implementation

`loop()` is repeatedly called after `setup()` finishes.

In this project, `loop()` mainly:

- Toggles the onboard LED as a heartbeat.
- Prints Raw data if debug output is enabled.
- Reads the current E0/E1 touch states.
- Updates Teensy PIN1/PIN2 HIGH/LOW outputs.
- Prints touched/released events when touch states change.

The overall flow can be understood as:

```text
Power on
  -> Arduino/Teensy framework initialization
  -> Run setup() once
  -> Repeatedly run loop()
       -> Read sensor
       -> Determine touch state
       -> Update output pins
       -> Print events or debug information
```
