#include <Wire.h>
#include <Adafruit_MPR121.h>

// Wiring for Teensy 4.1:
// MPR121 SDA  -> pin 18 / SDA0
// MPR121 SCL  -> pin 19 / SCL0
// MPR121 IRQ  -> digital pin 0
// MPR121 ADDR -> GND, so the I2C address is 0x5A

constexpr uint8_t MPR121_ADDR = 0x5A;
constexpr uint8_t IRQ_PIN = 0;
constexpr uint8_t CHANNEL_0_OUTPUT_PIN = 1;
constexpr uint8_t CHANNEL_1_OUTPUT_PIN = 2;
constexpr uint8_t LED_PIN = LED_BUILTIN;
constexpr uint8_t ELECTRODE_COUNT = 2;

// Tune these if your electrodes are too sensitive or not sensitive enough.
// Lower values are more sensitive; higher values are less sensitive.
constexpr uint8_t TOUCH_THRESHOLD = 5;
constexpr uint8_t RELEASE_THRESHOLD = 3;
constexpr uint32_t DEBUG_PRINT_INTERVAL_MS = 1000;
constexpr uint8_t CALIBRATION_SAMPLES = 20;
constexpr bool ENABLE_DEBUG_OUTPUT = false;
constexpr bool ENABLE_TOUCH_MASK_OUTPUT = false;

Adafruit_MPR121 cap = Adafruit_MPR121();

volatile bool touchInterrupt = false;
uint16_t lastTouched = 0;
uint16_t softwareBaseline[ELECTRODE_COUNT] = {};

void onTouchInterrupt() {
  touchInterrupt = true;
}

void printTouchedChannels(uint16_t touched) {
  bool anyTouched = false;

  Serial.print("Touched mask: 0b");
  for (int8_t i = ELECTRODE_COUNT - 1; i >= 0; --i) {
    Serial.print((touched & (1 << i)) ? '1' : '0');
  }

  Serial.print(" | channels:");
  for (uint8_t i = 0; i < ELECTRODE_COUNT; ++i) {
    if (touched & (1 << i)) {
      Serial.print(' ');
      Serial.print(i);
      anyTouched = true;
    }
  }

  if (!anyTouched) {
    Serial.print(" none");
  }

  Serial.println();
}

void updateTouchOutputPins(uint16_t touched) {
  digitalWrite(CHANNEL_0_OUTPUT_PIN, (touched & (1 << 0)) ? HIGH : LOW);
  digitalWrite(CHANNEL_1_OUTPUT_PIN, (touched & (1 << 1)) ? HIGH : LOW);
}

void calibrateSoftwareBaseline() {
  Serial.println("Calibrating software baseline. Do not touch electrodes...");

  for (uint8_t sample = 0; sample < CALIBRATION_SAMPLES; ++sample) {
    for (uint8_t i = 0; i < ELECTRODE_COUNT; ++i) {
      softwareBaseline[i] += cap.filteredData(i);
    }
    delay(25);
  }

  Serial.print("Software baseline:");
  for (uint8_t i = 0; i < ELECTRODE_COUNT; ++i) {
    softwareBaseline[i] /= CALIBRATION_SAMPLES;
    Serial.print(' ');
    Serial.print(i);
    Serial.print('=');
    Serial.print(softwareBaseline[i]);
  }
  Serial.println();
}

uint16_t readSoftwareTouched() {
  uint16_t touched = 0;

  for (uint8_t i = 0; i < ELECTRODE_COUNT; ++i) {
    const uint16_t filtered = cap.filteredData(i);
    const int16_t delta = softwareBaseline[i] - filtered;
    const uint16_t mask = (1 << i);
    const bool wasTouched = lastTouched & mask;
    const uint8_t threshold = wasTouched ? RELEASE_THRESHOLD : TOUCH_THRESHOLD;

    if (delta >= threshold) {
      touched |= mask;
    }
  }

  return touched;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    // Give USB Serial a moment to connect, but do not block forever.
  }

  Serial.println("MPR121 multi-channel touch test starting...");

  pinMode(IRQ_PIN, INPUT_PULLUP);
  pinMode(CHANNEL_0_OUTPUT_PIN, OUTPUT);
  pinMode(CHANNEL_1_OUTPUT_PIN, OUTPUT);
  digitalWrite(CHANNEL_0_OUTPUT_PIN, LOW);
  digitalWrite(CHANNEL_1_OUTPUT_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);

  Wire.begin();             // Teensy 4.1 Wire uses SDA0 pin 18 and SCL0 pin 19.
  Wire.setClock(400000);    // MPR121 supports fast-mode I2C.

  if (!cap.begin(MPR121_ADDR, &Wire)) {
    Serial.println("MPR121 not found at 0x5A. Check SDA/SCL/ADDR/power wiring.");
    while (true) {
      delay(100);
    }
  }

  cap.setThresholds(TOUCH_THRESHOLD, RELEASE_THRESHOLD);
  calibrateSoftwareBaseline();

  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), onTouchInterrupt, FALLING);

  lastTouched = readSoftwareTouched();
  updateTouchOutputPins(lastTouched);
  if (ENABLE_TOUCH_MASK_OUTPUT) {
    printTouchedChannels(lastTouched);
  }

  Serial.println("Ready. Touch one or more electrodes.");
  if (ENABLE_DEBUG_OUTPUT) {
    Serial.println("Debug columns: channel: filtered/softwareBaseline/delta. Delta should rise when touched.");
  }
}

void loop() {
  // IRQ is active-low and fires when touch status changes. The timed fallback
  // keeps output responsive even if the interrupt was missed during startup.
  static uint32_t lastPollMs = 0;
  static uint32_t lastDebugPrintMs = 0;
  static uint32_t lastLedToggleMs = 0;
  const uint32_t now = millis();

  if (now - lastLedToggleMs >= 500) {
    lastLedToggleMs = now;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }

  if (ENABLE_DEBUG_OUTPUT && now - lastDebugPrintMs >= DEBUG_PRINT_INTERVAL_MS) {
    lastDebugPrintMs = now;
    Serial.print("Raw: ");
    for (uint8_t i = 0; i < ELECTRODE_COUNT; ++i) {
      const uint16_t filtered = cap.filteredData(i);
      const uint16_t baseline = softwareBaseline[i];
      const int16_t delta = baseline - filtered;

      Serial.print(i);
      Serial.print(':');
      Serial.print(filtered);
      Serial.print('/');
      Serial.print(baseline);
      Serial.print('/');
      Serial.print(delta);
      if (i + 1 < ELECTRODE_COUNT) {
        Serial.print("  ");
      }
    }
    Serial.println();
  }

  if (!touchInterrupt && now - lastPollMs < 50) {
    return;
  }

  noInterrupts();
  touchInterrupt = false;
  interrupts();

  lastPollMs = now;

  const uint16_t touched = readSoftwareTouched();
  updateTouchOutputPins(touched);
  if (touched != lastTouched) {
    for (uint8_t i = 0; i < ELECTRODE_COUNT; ++i) {
      const uint16_t mask = (1 << i);
      if ((touched & mask) && !(lastTouched & mask)) {
        Serial.print("Electrode ");
        Serial.print(i);
        Serial.println(" touched");
      }
      if (!(touched & mask) && (lastTouched & mask)) {
        Serial.print("Electrode ");
        Serial.print(i);
        Serial.println(" released");
      }
    }

    if (ENABLE_TOUCH_MASK_OUTPUT) {
      printTouchedChannels(touched);
    }
    lastTouched = touched;
  }
}
