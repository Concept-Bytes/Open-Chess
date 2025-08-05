#ifndef SENSOR_TEST_H
#define SENSOR_TEST_H

#include "board_driver.h"

// ---------------------------
// Sensor Test Mode Class
// ---------------------------
class SensorTest {
private:
    BoardDriver* boardDriver;
    
    // Expected initial configuration for testing
    static const char INITIAL_BOARD[8][8];

public:
    SensorTest(BoardDriver* bd);
    void begin();
    void update();
    bool isActive();
    void reset();
};

#endif // SENSOR_TEST_H