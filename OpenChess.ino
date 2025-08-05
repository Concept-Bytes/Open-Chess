#include "board_driver.h"
#include "chess_engine.h"
#include "chess_moves.h"
#include "sensor_test.h"
#include "chess_bot.h"

// Uncomment the next line to enable WiFi features (requires compatible board)
#define ENABLE_WIFI  // Currently disabled - RP2040 boards use local mode only
#ifdef ENABLE_WIFI
  // Use different WiFi manager based on board type
  #if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_NANO_RP2040_CONNECT)
    #include "wifi_manager.h"  // Full WiFi implementation for boards with WiFiNINA
  #else
    #include "wifi_manager_rp2040.h"  // Placeholder for RP2040 and other boards
    #define WiFiManager WiFiManagerRP2040
  #endif
#endif

// ---------------------------
// Game State and Configuration
// ---------------------------

// Game Mode Definitions
enum GameMode {
  MODE_SELECTION = 0,
  MODE_CHESS_MOVES = 1,
  MODE_CHESS_BOT = 2,      // Chess vs Bot mode
  MODE_GAME_3 = 3,         // Reserved for future game mode
  MODE_SENSOR_TEST = 4
};

// Global instances
BoardDriver boardDriver;
ChessEngine chessEngine;
ChessMoves chessMoves(&boardDriver, &chessEngine);
SensorTest sensorTest(&boardDriver);
ChessBot chessBot(&boardDriver, &chessEngine, BOT_MEDIUM);

#ifdef ENABLE_WIFI
WiFiManager wifiManager;
#endif

// Current game state
GameMode currentMode = MODE_SELECTION;
bool modeInitialized = false;

// ---------------------------
// Function Prototypes
// ---------------------------
void showGameSelection();
void handleGameSelection();
void initializeSelectedMode(GameMode mode);

// ---------------------------
// SETUP
// ---------------------------
void setup() {
  // Initialize Serial with extended timeout
  Serial.begin(9600);
  
  // Wait for Serial to be ready (critical for RP2040)
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime < 10000)) {
    // Wait up to 10 seconds for Serial connection
    delay(100);
  }
  
  // Force a delay to ensure Serial is stable
  delay(2000);
  
  Serial.println();
  Serial.println("================================================");
  Serial.println("         OpenChess Starting Up");
  Serial.println("================================================");
  Serial.println("DEBUG: Serial communication established");
  Serial.print("DEBUG: Millis since boot: ");
  Serial.println(millis());
  
  // Debug board type detection
  Serial.println("DEBUG: Board type detection:");
  #if defined(ARDUINO_SAMD_MKRWIFI1010)
  Serial.println("  - Detected: ARDUINO_SAMD_MKRWIFI1010");
  #elif defined(ARDUINO_SAMD_NANO_33_IOT)
  Serial.println("  - Detected: ARDUINO_SAMD_NANO_33_IOT");
  #elif defined(ARDUINO_NANO_RP2040_CONNECT)
  Serial.println("  - Detected: ARDUINO_NANO_RP2040_CONNECT");
  #else
  Serial.println("  - Detected: Unknown/Other board type");
  #endif
  
  // Check which mode is compiled
#ifdef ENABLE_WIFI
  Serial.println("DEBUG: Compiled with ENABLE_WIFI defined");
#else
  Serial.println("DEBUG: Compiled without ENABLE_WIFI (local mode only)");
#endif

  Serial.println("DEBUG: About to initialize board driver...");
  // Initialize board driver
  boardDriver.begin();
  Serial.println("DEBUG: Board driver initialized successfully");

#ifdef ENABLE_WIFI
  Serial.println();
  Serial.println("=== WiFi Mode Enabled ===");
  Serial.println("DEBUG: About to initialize WiFi Manager...");
  Serial.println("DEBUG: This will attempt to create Access Point");
  
  // Initialize WiFi Manager
  wifiManager.begin();
  
  Serial.println("DEBUG: WiFi Manager initialization completed");
  Serial.println("If WiFi AP was created successfully, you should see:");
  Serial.println("- Network name: OpenChessBoard");
  Serial.println("- Password: chess123");
  Serial.println("- Web interface: http://192.168.4.1");
  Serial.println("Or place a piece on the board for local selection");
#else
  Serial.println();
  Serial.println("=== Local Mode Only ===");
  Serial.println("WiFi features are disabled in this build");
  Serial.println("To enable WiFi: Uncomment #define ENABLE_WIFI and recompile");
#endif

  Serial.println();
  Serial.println("=== Game Selection Mode ===");
  Serial.println("DEBUG: About to show game selection LEDs...");

  // Show game selection interface
  showGameSelection();
  
  Serial.println("DEBUG: Game selection LEDs should now be visible");
  Serial.println("Four white LEDs should be lit in the center of the board:");
  Serial.println("Position 1 (3,3): Chess Moves (Human vs Human)");
  Serial.println("Position 2 (3,4): Chess Bot (Human vs AI)");
  Serial.println("Position 3 (4,3): Game Mode 3 (Coming Soon)");
  Serial.println("Position 4 (4,4): Sensor Test");
  Serial.println();
  Serial.println("Place any chess piece on a white LED to select that mode");
  Serial.println("================================================");
  Serial.println("         Setup Complete - Entering Main Loop");
  Serial.println("================================================");
}

// ---------------------------
// MAIN LOOP
// ---------------------------
void loop() {
  static unsigned long lastDebugPrint = 0;
  static bool firstLoop = true;
  
  if (firstLoop) {
    Serial.println("DEBUG: Entered main loop - system is running");
    firstLoop = false;
  }
  
  // Print periodic status every 10 seconds
  if (millis() - lastDebugPrint > 10000) {
    Serial.print("DEBUG: Loop running, uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    lastDebugPrint = millis();
  }

#ifdef ENABLE_WIFI
  // Handle WiFi clients
  wifiManager.handleClient();
  
  // Check for WiFi game selection
  int selectedMode = wifiManager.getSelectedGameMode();
  if (selectedMode > 0) {
    Serial.print("DEBUG: WiFi game selection detected: ");
    Serial.println(selectedMode);
    
    switch (selectedMode) {
      case 1:
        currentMode = MODE_CHESS_MOVES;
        break;
      case 4:
        currentMode = MODE_SENSOR_TEST;
        break;
      default:
        Serial.println("Invalid game mode selected via WiFi");
        selectedMode = 0;
        break;
    }
    
    if (selectedMode > 0) {
      modeInitialized = false;
      boardDriver.clearAllLEDs();
      wifiManager.resetGameSelection();
      
      // Brief confirmation animation
      for (int i = 0; i < 3; i++) {
        boardDriver.setSquareLED(3, 3, 0, 255, 0, 0); // Green flash
        boardDriver.setSquareLED(3, 4, 0, 255, 0, 0);
        boardDriver.setSquareLED(4, 3, 0, 255, 0, 0);
        boardDriver.setSquareLED(4, 4, 0, 255, 0, 0);
        boardDriver.showLEDs();
        delay(200);
        boardDriver.clearAllLEDs();
        delay(200);
      }
    }
  }
#endif

  if (currentMode == MODE_SELECTION) {
    handleGameSelection();
  } else {
    static bool modeChangeLogged = false;
    if (!modeChangeLogged) {
      Serial.print("DEBUG: Mode changed to: ");
      Serial.println(currentMode);
      modeChangeLogged = true;
    }
    // Initialize the selected mode if not already done
    if (!modeInitialized) {
      initializeSelectedMode(currentMode);
      modeInitialized = true;
    }
    
    // Run the current game mode
    switch (currentMode) {
      case MODE_CHESS_MOVES:
        chessMoves.update();
        break;
      case MODE_CHESS_BOT:
        chessBot.update();
        break;
      case MODE_SENSOR_TEST:
        sensorTest.update();
        break;
      case MODE_GAME_3:
        // Future game modes - placeholder
        Serial.println("Game mode coming soon!");
        delay(1000);
        break;
      default:
        currentMode = MODE_SELECTION;
        modeInitialized = false;
        showGameSelection();
        break;
    }
  }
  
  delay(50); // Small delay to prevent overwhelming the system
}

// ---------------------------
// GAME SELECTION FUNCTIONS
// ---------------------------

void showGameSelection() {
  // Clear all LEDs first
  boardDriver.clearAllLEDs();
  
  // Light up the 4 selector positions in the middle of the board
  // All positions now use bright white for better visibility
  // Position 1: Chess Moves (row 3, col 3) - White
  boardDriver.setSquareLED(3, 3, 0, 0, 0, 255);
  
  // Position 2: Game Mode 2 (row 3, col 4) - White
  boardDriver.setSquareLED(3, 4, 0, 0, 0, 255);
  
  // Position 3: Game Mode 3 (row 4, col 3) - White
  boardDriver.setSquareLED(4, 3, 0, 0, 0, 255);
  
  // Position 4: Sensor Test (row 4, col 4) - White
  boardDriver.setSquareLED(4, 4, 0, 0, 0, 255);
  
  boardDriver.showLEDs();
}

void handleGameSelection() {
  boardDriver.readSensors();
  
  // Check for piece placement on selector squares
  if (boardDriver.getSensorState(3, 3)) {
    // Chess Moves selected
    Serial.println("Chess Moves mode selected!");
    currentMode = MODE_CHESS_MOVES;
    modeInitialized = false;
    boardDriver.clearAllLEDs();
    delay(500); // Debounce delay
  }
  else if (boardDriver.getSensorState(3, 4)) {
    // Chess Bot selected
    Serial.println("Chess Bot mode selected (Human vs AI)!");
    currentMode = MODE_CHESS_BOT;
    modeInitialized = false;
    boardDriver.clearAllLEDs();
    delay(500);
  }
  else if (boardDriver.getSensorState(4, 3)) {
    // Game Mode 3 selected
    Serial.println("Game Mode 3 selected (Coming Soon)!");
    currentMode = MODE_GAME_3;
    modeInitialized = false;
    boardDriver.clearAllLEDs();
    delay(500);
  }
  else if (boardDriver.getSensorState(4, 4)) {
    // Sensor Test selected
    Serial.println("Sensor Test mode selected!");
    currentMode = MODE_SENSOR_TEST;
    modeInitialized = false;
    boardDriver.clearAllLEDs();
    delay(500);
  }
  
  delay(100);
}

void initializeSelectedMode(GameMode mode) {
  switch (mode) {
    case MODE_CHESS_MOVES:
      Serial.println("Starting Chess Moves (Human vs Human)...");
      chessMoves.begin();
      break;
    case MODE_CHESS_BOT:
      Serial.println("Starting Chess Bot (Human vs AI)...");
      chessBot.begin();
      break;
    case MODE_SENSOR_TEST:
      Serial.println("Starting Sensor Test...");
      sensorTest.begin();
      break;
    case MODE_GAME_3:
      Serial.println("This game mode will be available in a future update!");
      Serial.println("Returning to game selection in 3 seconds...");
      delay(3000);
      currentMode = MODE_SELECTION;
      modeInitialized = false;
      showGameSelection();
      break;
    default:
      currentMode = MODE_SELECTION;
      modeInitialized = false;
      showGameSelection();
      break;
  }
}
