# Open Chess

Open Chess is an Arduino-based chessboard project that integrates an 8×8 NeoPixel matrix, hall effect sensors, and a 74HC594 shift register to both display a chess game and detect piece movement. The project implements basic chess gameplay with visual feedback (e.g., blinking, pulsing, and animated effects) and includes a sensor test sketch for verifying the board’s sensor grid functionality.

## Overview

Open Chess uses an Arduino to control a NeoPixel matrix that displays the chess board. Hall effect sensors detect magnets embedded in each chess piece, allowing the board to sense when a piece is picked up or placed. The main sketch simulates moves using a finite state machine (FSM) that provides visual feedback (red blinking for selected pieces and white overlays for possible moves) and even features a dynamic firework animation when the game starts.

Additionally, the repository includes a `Sensor_Test.ino` sketch, which simply lights up the tiles corresponding to the starting pieces. This is an excellent tool for verifying your sensor grid and LED mapping before running the full chess game code.

## 📊 Finite State Machine Diagram

This diagram shows the core logic of how Open Chess detects and handles game interactions using a finite state machine:

```mermaid
stateDiagram-v2
    [*] --> Idle

    Idle --> PieceSelected : Piece picked up
    PieceSelected --> ShowMoves : Show possible moves

    ShowMoves --> MoveConfirmed : Piece placed on valid square
    ShowMoves --> InvalidMove : Piece placed on invalid square

    MoveConfirmed --> ReturnToIdle : Update board
    InvalidMove --> ReturnToIdle : Reset piece

    ReturnToIdle --> Idle

    Idle --> VictoryAnimation : Game started
    MoveConfirmed --> VictoryAnimation : Game won

    VictoryAnimation --> Idle : Animation ends
```

## Hardware Requirements

- **Arduino Board:** (e.g., Arduino Nano RP2040, Arduino Uno, or compatible)
- **NeoPixel Matrix:** 8×8 NeoPixel LED array (using GRBW LEDs)
- **Shift Register:** 74HC594 for driving the sensor matrix
- **Hall Effect Sensors:** One per board square to detect magnets embedded in chess pieces
- **Magnets:** Embedded in each chess piece
- **Connecting Wires and Breadboard** for prototyping

## Libraries and Dependencies

- [Adafruit NeoPixel Library](https://github.com/adafruit/Adafruit_NeoPixel)
- Standard Arduino libraries (e.g., `math.h` is used for animations)

Make sure to install the Adafruit NeoPixel Library via the Arduino Library Manager before compiling.

## Files in the Repository

- **OpenChess.ino (or main sketch file):**  
  Contains the full chess game logic including piece movement, sensor reading, move indication (red blinking and white move overlays), and animations (like the firework effect). This is the final draft for a standard game of chess.

- **Sensor_Test.ino:**  
  A simplified sketch that lights up the tiles corresponding to the starting pieces on the chess board. This is useful for verifying your sensor grid and LED mapping before running the full game.

## How to Use

1. **Setup Hardware:**  
   Connect your NeoPixel matrix to the designated LED pin (configured as pin 17 in the code). Wire the sensors through the 74HC594 shift register using the defined pins (SER_PIN, SRCLK_PIN, RCLK_PIN) and connect the column sensors to digital pins D6–D13.

2. **Install Required Libraries:**  
   In the Arduino IDE, go to **Sketch > Include Library > Manage Libraries...** and install the Adafruit NeoPixel library.

3. **Upload Sensor Test (Optional):**  
   Upload the `Sensor_Test.ino` sketch to verify that the sensors correctly light up the board squares where pieces are expected. This step is recommended before running the full game code.

4. **Upload the Full Open Chess Code:**  
   Once you have confirmed that the sensor test works, upload the main Open Chess sketch. The game will wait until the board is set up (i.e., sensors indicate that all expected pieces are in place) and then run the startup sequence (which includes a firework animation). During gameplay, piece pickups and placements are detected via the sensors, and moves are visually indicated by blinking the active square and showing possible moves.

5. **Enjoy Open Chess:**  
   The board will provide visual feedback (using blinking and pulsing effects) for both white and black moves. The code simulates piece moves and provides basic gameplay functionality.

## How It Works

- **Sensor Reading:**  
  Open Chess scans the hall effect sensors (via a shift register) to detect which board squares have a chess piece (a magnet is present). It compares the current sensor state with the previous state to detect piece pickups or placements.

- **Board Mapping:**  
  The chess board is represented as an 8×8 character array. The LED mapping function converts board coordinates into a NeoPixel index using the formula:  
  `index = col * NUM_COLS + (7 - row)`

- **Move Indication and Animations:**  
  When a piece is picked up, the corresponding square blinks red and the legal moves for that piece are displayed in white. The code uses a FSM to simulate moves (e.g., white pawn, black pawn, knight, bishop, etc.) and features a firework animation to indicate game startup.

## Customization

Feel free to modify:
- The FSM to implement your own move logic or additional rules.
- The visual effects (e.g., blinking intervals, firework animation parameters).
- Hardware pin assignments if using a different Arduino board or wiring configuration.

## Contributing

Contributions are welcome! If you have ideas for improvements or additional features, please fork the repository and submit a pull request.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

Happy coding and enjoy playing chess with Open Chess!
