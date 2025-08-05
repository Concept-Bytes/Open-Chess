#include <Adafruit_NeoPixel.h>

// ---------------------------
// NeoPixel Setup
// ---------------------------
#define LED_PIN     17       // Pin for NeoPixels
#define NUM_ROWS    8
#define NUM_COLS    8
#define LED_COUNT   (NUM_ROWS * NUM_COLS)
#define BRIGHTNESS  100

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// ---------------------------
// Shift Register (74HC594) Pins
// ---------------------------
#define SER_PIN     2   // Serial data input  (74HC594 pin 14)
#define SRCLK_PIN   3   // Shift register clock (pin 11)
#define RCLK_PIN    4   // Latch clock (pin 12)
// Pin 13 (OE) on 74HC594 must be tied HIGH (active-high).
// Pin 10 (SRCLR) on 74HC594 must be tied HIGH if not used for clearing.

// ---------------------------
// Column Input Pins (D6..D13)
// ---------------------------
int colPins[NUM_COLS] = {6, 7, 8, 9, 10, 11, 12, 13};

// ---------------------------
// Row Patterns (LSB-first)
// ---------------------------
byte rowPatterns[8] = {
  0x01, // Row 0
  0x02, // Row 1
  0x04, // Row 2
  0x08, // Row 3
  0x10, // Row 4
  0x20, // Row 5
  0x40, // Row 6
  0x80  // Row 7
};

// ---------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // NeoPixel init
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(BRIGHTNESS);

  // Shift register control pins
  pinMode(SER_PIN,   OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);
  pinMode(RCLK_PIN,  OUTPUT);

  // Column pins as inputs
  for (int c = 0; c < NUM_COLS; c++) {
    pinMode(colPins[c], INPUT);
  }

  // Initialize shift register to all LOW (no line active)
  loadShiftRegister(0x00);
}

// ---------------------------------------------------------------------
// LOOP
// ---------------------------------------------------------------------
void loop() {
  // Clear all LEDs
  for(int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 0); // Off
  }

  // Scan each row
  for (int row = 0; row < NUM_ROWS; row++) {

    // 1) Enable this row via the shift register
    loadShiftRegister(rowPatterns[row]);

    // Small delay to let signals settle
    delayMicroseconds(100);

    // 2) Read all columns
    for (int c = 0; c < NUM_COLS; c++) {
      int sensorVal = digitalRead(colPins[c]);

      // If sensor is active (LOW):
      if (sensorVal == LOW) {
        // Map row, col to the correct NeoPixel index, no flips:
        int pixelIndex = c * NUM_COLS + (7-row);

        // Set that pixel to white for RGBW
        strip.setPixelColor(pixelIndex, strip.Color(0, 0, 0, 255));
      }
    }
  }

  // Turn off the row lines
  loadShiftRegister(0x00);

  // Update NeoPixels
  strip.show();

  // Small pause
  delay(100);
}

// ---------------------------------------------------------------------
// SHIFT OUT  (LSB-first to match rowPatterns[])
// ---------------------------------------------------------------------
void loadShiftRegister(byte data) {
  digitalWrite(RCLK_PIN, LOW);

  for (int i = 0; i < 8; i++) {
    bool bitVal = (data & (1 << i)) != 0;
    digitalWrite(SER_PIN, bitVal ? HIGH : LOW);

    digitalWrite(SRCLK_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(SRCLK_PIN, LOW);
    delayMicroseconds(10);
  }

  digitalWrite(RCLK_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(RCLK_PIN, LOW);
}