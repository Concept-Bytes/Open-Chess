#include "sensor_test.h"
#include <Arduino.h>

// Expected initial configuration for sensor testing
const char SensorTest::INITIAL_BOARD[8][8] = {
  {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},  // row 0 (rank 1)
  {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},  // row 1 (rank 2)
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},    // row 2 (rank 3)
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},    // row 3 (rank 4)
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},    // row 4 (rank 5)
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},    // row 5 (rank 6)
  {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},    // row 6 (rank 7)
  {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}     // row 7 (rank 8)
};

SensorTest::SensorTest(BoardDriver* bd) : boardDriver(bd) {
}

void SensorTest::begin() {
    Serial.println("Starting Sensor Test Mode...");
    Serial.println("Place pieces on the board to see them light up!");
    Serial.println("This mode continuously displays detected pieces.");
    
    boardDriver->clearAllLEDs();
}

void SensorTest::update() {
    // Read current sensor state
    boardDriver->readSensors();
    
    // Clear all LEDs first
    boardDriver->clearAllLEDs();
    
    // Light up squares where pieces are detected
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (boardDriver->getSensorState(row, col)) {
                // Light up detected pieces in white
                boardDriver->setSquareLED(row, col, 0, 0, 0, 255);
            }
        }
    }
    
    // Show the updated LED state
    boardDriver->showLEDs();
    
    // Print board state periodically for debugging
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 2000) { // Print every 2 seconds
        boardDriver->printBoardState(INITIAL_BOARD);
        lastPrint = millis();
    }
    
    delay(100); // Small delay to prevent overwhelming the system
}

bool SensorTest::isActive() {
    return true; // Always active once started
}

void SensorTest::reset() {
    boardDriver->clearAllLEDs();
    Serial.println("Sensor test reset - ready for testing!");
}