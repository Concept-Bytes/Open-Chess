#include "board_driver.h"
#include <math.h>

// ---------------------------
// BoardDriver Implementation
// ---------------------------

BoardDriver::BoardDriver() : strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800) {
    // Initialize column pins
    int tempColPins[NUM_COLS] = COL_PINS;
    for (int i = 0; i < NUM_COLS; i++) {
        colPins[i] = tempColPins[i];
    }
    
    // Initialize row patterns (LSB-first for shift register)
    rowPatterns[0] = 0x01; // row 0
    rowPatterns[1] = 0x02; // row 1
    rowPatterns[2] = 0x04; // row 2
    rowPatterns[3] = 0x08; // row 3
    rowPatterns[4] = 0x10; // row 4
    rowPatterns[5] = 0x20; // row 5
    rowPatterns[6] = 0x40; // row 6
    rowPatterns[7] = 0x80; // row 7
}

void BoardDriver::begin() {
    // Initialize NeoPixel strip
    strip.begin();
    strip.show(); // turn off all pixels
    strip.setBrightness(BRIGHTNESS);

    // Setup shift register control pins
    pinMode(SER_PIN,   OUTPUT);
    pinMode(SRCLK_PIN, OUTPUT);
    pinMode(RCLK_PIN,  OUTPUT);

    // Setup column input pins
    for (int c = 0; c < NUM_COLS; c++) {
        pinMode(colPins[c], INPUT);
    }

    // Initialize shift register to no row active
    loadShiftRegister(0x00);
    
    // Initialize sensor arrays
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            sensorState[row][col] = false;
            sensorPrev[row][col] = false;
        }
    }
}

void BoardDriver::loadShiftRegister(byte data) {
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

void BoardDriver::readSensors() {
    for (int row = 0; row < 8; row++) {
        loadShiftRegister(rowPatterns[row]);
        delayMicroseconds(100);
        for (int col = 0; col < NUM_COLS; col++) {
            int sensorVal = digitalRead(colPins[col]);
            sensorState[row][col] = (sensorVal == LOW);
        }
    }
    loadShiftRegister(0x00);
}

bool BoardDriver::getSensorState(int row, int col) {
    return sensorState[row][col];
}

bool BoardDriver::getSensorPrev(int row, int col) {
    return sensorPrev[row][col];
}

void BoardDriver::updateSensorPrev() {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            sensorPrev[row][col] = sensorState[row][col];
        }
    }
}

int BoardDriver::getPixelIndex(int row, int col) {
    return col * NUM_COLS + (7 - row);
}

void BoardDriver::clearAllLEDs() {
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, 0);
    }
    strip.show();
}

void BoardDriver::setSquareLED(int row, int col, uint32_t color) {
    int pixelIndex = getPixelIndex(row, col);
    strip.setPixelColor(pixelIndex, color);
}

void BoardDriver::setSquareLED(int row, int col, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    int pixelIndex = getPixelIndex(row, col);
    strip.setPixelColor(pixelIndex, strip.Color(r, g, b, w));
}

void BoardDriver::showLEDs() {
    strip.show();
}

void BoardDriver::highlightSquare(int row, int col, uint32_t color) {
    setSquareLED(row, col, color);
    showLEDs();
}

void BoardDriver::blinkSquare(int row, int col, int times) {
    int pixelIndex = getPixelIndex(row, col);
    for (int i = 0; i < times; i++) {
        strip.setPixelColor(pixelIndex, strip.Color(0, 0, 0, 255));
        strip.show();
        delay(200);
        strip.setPixelColor(pixelIndex, 0);
        strip.show();
        delay(200);
    }
}

void BoardDriver::fireworkAnimation() {
    float centerX = 3.5;
    float centerY = 3.5;
    
    // Expansion phase:
    for (float radius = 0; radius < 6; radius += 0.5) {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                float dx = col - centerX;
                float dy = row - centerY;
                float dist = sqrt(dx * dx + dy * dy);
                int pixelIndex = getPixelIndex(row, col);
                if (fabs(dist - radius) < 0.5)
                    strip.setPixelColor(pixelIndex, strip.Color(0, 0, 0, 255));
                else
                    strip.setPixelColor(pixelIndex, 0);
            }
        }
        strip.show();
        delay(100);
    }
    
    // Contraction phase:
    for (float radius = 6; radius > 0; radius -= 0.5) {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                float dx = col - centerX;
                float dy = row - centerY;
                float dist = sqrt(dx * dx + dy * dy);
                int pixelIndex = getPixelIndex(row, col);
                if (fabs(dist - radius) < 0.5)
                    strip.setPixelColor(pixelIndex, strip.Color(0, 0, 0, 255));
                else
                    strip.setPixelColor(pixelIndex, 0);
            }
        }
        strip.show();
        delay(100);
    }
    
    // Second expansion phase:
    for (float radius = 0; radius < 6; radius += 0.5) {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                float dx = col - centerX;
                float dy = row - centerY;
                float dist = sqrt(dx * dx + dy * dy);
                int pixelIndex = getPixelIndex(row, col);
                if (fabs(dist - radius) < 0.5)
                    strip.setPixelColor(pixelIndex, strip.Color(0, 0, 0, 255));
                else
                    strip.setPixelColor(pixelIndex, 0);
            }
        }
        strip.show();
        delay(100);
    }
    
    // Clear all LEDs
    clearAllLEDs();
}

void BoardDriver::captureAnimation() {
    float centerX = 3.5;
    float centerY = 3.5;
    
    // Pulsing outward animation
    for (int pulse = 0; pulse < 3; pulse++) {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                float dx = col - centerX;
                float dy = row - centerY;
                float dist = sqrt(dx * dx + dy * dy);
                
                // Create a pulsing effect around the center
                float pulseWidth = 1.5 + pulse;
                int pixelIndex = getPixelIndex(row, col);
                
                if (dist >= pulseWidth - 0.5 && dist <= pulseWidth + 0.5) {
                    // Alternate between red and orange for capture effect
                    uint32_t color = (pulse % 2 == 0) 
                        ? strip.Color(255, 0, 0, 0)   // Red
                        : strip.Color(255, 165, 0, 0); // Orange
                    strip.setPixelColor(pixelIndex, color);
                } else {
                    strip.setPixelColor(pixelIndex, 0);
                }
            }
        }
        strip.show();
        delay(150);
    }
    
    // Clear LEDs
    clearAllLEDs();
}

void BoardDriver::promotionAnimation(int col) {
    const uint32_t PROMOTION_COLOR = strip.Color(255, 215, 0, 50); // Gold with white
    
    // Column-based waterfall animation
    for (int step = 0; step < 16; step++) {
        for (int row = 0; row < 8; row++) {
            int pixelIndex = getPixelIndex(row, col);
            
            // Create a golden wave moving up and down the column
            if ((step + row) % 8 < 4) {
                strip.setPixelColor(pixelIndex, PROMOTION_COLOR);
            } else {
                strip.setPixelColor(pixelIndex, 0);
            }
        }
        strip.show();
        delay(100);
    }
    
    // Clear the animation
    for (int row = 0; row < 8; row++) {
        int pixelIndex = getPixelIndex(row, col);
        strip.setPixelColor(pixelIndex, 0);
    }
    strip.show();
}

bool BoardDriver::checkInitialBoard(const char initialBoard[8][8]) {
    readSensors();
    bool allPresent = true;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (initialBoard[row][col] != ' ' && !sensorState[row][col]) {
                allPresent = false;
            }
        }
    }
    return allPresent;
}

void BoardDriver::updateSetupDisplay(const char initialBoard[8][8]) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int pixelIndex = getPixelIndex(row, col);
            if (initialBoard[row][col] != ' ' && sensorState[row][col]) {
                strip.setPixelColor(pixelIndex, strip.Color(0, 0, 0, 255));
            } else {
                strip.setPixelColor(pixelIndex, 0);
            }
        }
    }
    strip.show();
}

void BoardDriver::printBoardState(const char initialBoard[8][8]) {
    Serial.println("Current Board:");
    for (int row = 0; row < 8; row++) {
        Serial.print("{ ");
        for (int col = 0; col < 8; col++) {
            char displayChar = ' ';
            if (initialBoard[row][col] != ' ') {
                displayChar = sensorState[row][col] ? initialBoard[row][col] : '-';
            }
            Serial.print("'");
            Serial.print(displayChar);
            Serial.print("'");
            if (col < 7) Serial.print(", ");
        }
        Serial.println(" },");
    }
    Serial.println();
}
