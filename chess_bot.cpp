#include "chess_bot.h"
#include <Arduino.h>

ChessBot::ChessBot(BoardDriver* boardDriver, ChessEngine* chessEngine, BotDifficulty diff) {
    _boardDriver = boardDriver;
    _chessEngine = chessEngine;
    difficulty = diff;
    
    // Set difficulty settings
    switch(difficulty) {
        case BOT_EASY: settings = StockfishSettings::easy(); break;
        case BOT_MEDIUM: settings = StockfishSettings::medium(); break;
        case BOT_HARD: settings = StockfishSettings::hard(); break;
        case BOT_EXPERT: settings = StockfishSettings::expert(); break;
    }
    
    isWhiteTurn = true;
    gameStarted = false;
    botThinking = false;
    wifiConnected = false;
}

void ChessBot::begin() {
    Serial.println("=== Starting Chess Bot Mode ===");
    Serial.print("Bot Difficulty: ");
    
    switch(difficulty) {
        case BOT_EASY: Serial.println("Easy (Depth 6)"); break;
        case BOT_MEDIUM: Serial.println("Medium (Depth 10)"); break;
        case BOT_HARD: Serial.println("Hard (Depth 14)"); break;
        case BOT_EXPERT: Serial.println("Expert (Depth 16)"); break;
    }
    
    _boardDriver->clearAllLEDs();
    _boardDriver->showLEDs();
    
    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    showConnectionStatus();
    
    if (connectToWiFi()) {
        Serial.println("WiFi connected! Bot mode ready.");
        wifiConnected = true;
        
        // Show success animation
        for (int i = 0; i < 3; i++) {
            _boardDriver->clearAllLEDs();
            _boardDriver->showLEDs();
            delay(200);
            
            // Light up entire board green briefly
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    _boardDriver->setSquareLED(row, col, 0, 255, 0); // Green
                }
            }
            _boardDriver->showLEDs();
            delay(200);
        }
        
        initializeBoard();
        waitForBoardSetup();
    } else {
        Serial.println("Failed to connect to WiFi. Bot mode unavailable.");
        wifiConnected = false;
        
        // Show error animation (red flashing)
        for (int i = 0; i < 5; i++) {
            _boardDriver->clearAllLEDs();
            _boardDriver->showLEDs();
            delay(300);
            
            // Light up entire board red briefly
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    _boardDriver->setSquareLED(row, col, 255, 0, 0); // Red
                }
            }
            _boardDriver->showLEDs();
            delay(300);
        }
        
        _boardDriver->clearAllLEDs();
        _boardDriver->showLEDs();
    }
}

void ChessBot::update() {
    if (!wifiConnected) {
        return; // No WiFi, can't play against bot
    }
    
    if (!gameStarted) {
        return; // Waiting for initial setup
    }
    
    if (botThinking) {
        showBotThinking();
        return;
    }
    
    _boardDriver->readSensors();
    
    // Detect piece movements (player's turn - White pieces only)
    if (isWhiteTurn) {
        static unsigned long lastTurnDebug = 0;
        if (millis() - lastTurnDebug > 5000) {
            Serial.println("Your turn! Move a WHITE piece (uppercase letters)");
            lastTurnDebug = millis();
        }
        // Look for piece pickups and placements
        static int selectedRow = -1, selectedCol = -1;
        static bool piecePickedUp = false;
        
        // Check for piece pickup
        if (!piecePickedUp) {
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    if (!_boardDriver->getSensorState(row, col) && _boardDriver->getSensorPrev(row, col)) {
                        // Check what piece was picked up
                        char piece = board[row][col];
                        
                        if (piece != ' ') {
                            // Player should only be able to move White pieces (uppercase)
                            if (piece >= 'A' && piece <= 'Z') {
                            selectedRow = row;
                            selectedCol = col;
                            piecePickedUp = true;
                            
                            Serial.print("Player picked up WHITE piece '");
                            Serial.print(board[row][col]);
                            Serial.print("' at ");
                            Serial.print((char)('a' + col));
                            Serial.print(8 - row);
                            Serial.print(" (array position ");
                            Serial.print(row);
                            Serial.print(",");
                            Serial.print(col);
                            Serial.println(")");
                            
                            // Show selected square
                            _boardDriver->setSquareLED(row, col, 255, 0, 0); // Red
                            
                            // Show possible moves
                            int moveCount = 0;
                            int moves[27][2];
                            _chessEngine->getPossibleMoves(board, row, col, moveCount, moves);
                            
                            for (int i = 0; i < moveCount; i++) {
                                _boardDriver->setSquareLED(moves[i][0], moves[i][1], 255, 255, 255); // White
                            }
                            _boardDriver->showLEDs();
                            break;
                            } else {
                                // Player tried to pick up a Black piece - not allowed!
                                Serial.print("ERROR: You tried to pick up BLACK piece '");
                                Serial.print(piece);
                                Serial.print("' at ");
                                Serial.print((char)('a' + col));
                                Serial.print(8 - row);
                                Serial.println(". You can only move WHITE pieces!");
                                
                                // Flash red to indicate error
                                _boardDriver->blinkSquare(row, col, 3);
                            }
                        }
                    }
                }
                if (piecePickedUp) break;
            }
        }
        
        // Check for piece placement
        if (piecePickedUp) {
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    if (_boardDriver->getSensorState(row, col) && !_boardDriver->getSensorPrev(row, col)) {
                        // Check if piece was returned to its original position
                        if (row == selectedRow && col == selectedCol) {
                            // Piece returned to original position - cancel selection
                            Serial.println("Piece returned to original position. Selection cancelled.");
                            piecePickedUp = false;
                            selectedRow = selectedCol = -1;
                            
                            // Clear all indicators
                            _boardDriver->clearAllLEDs();
                            _boardDriver->showLEDs();
                            break;
                        }
                        
                        // Piece placed somewhere else - validate move
                        int moveCount = 0;
                        int moves[27][2];
                        _chessEngine->getPossibleMoves(board, selectedRow, selectedCol, moveCount, moves);
                        
                        bool validMove = false;
                        for (int i = 0; i < moveCount; i++) {
                            if (moves[i][0] == row && moves[i][1] == col) {
                                validMove = true;
                                break;
                            }
                        }
                        
                        if (validMove) {
                            char piece = board[selectedRow][selectedCol];
                            
                            // Complete LED animations BEFORE API request
                            processPlayerMove(selectedRow, selectedCol, row, col, piece);
                            
                            // Flash confirmation on destination square for player move
                            confirmSquareCompletion(row, col);
                            
                            piecePickedUp = false;
                            selectedRow = selectedCol = -1;
                            
                            // Switch to bot's turn
                            isWhiteTurn = false;
                            botThinking = true;
                            
                            Serial.println("Player move completed. Bot thinking...");
                            
                            // Start bot move calculation
                            makeBotMove();
                        } else {
                            Serial.println("Invalid move! Please try again.");
                            _boardDriver->blinkSquare(row, col, 3); // Blink red for invalid move
                            
                            // Restore move indicators - piece is still selected
                            _boardDriver->clearAllLEDs();
                            
                            // Show selected square again
                            _boardDriver->setSquareLED(selectedRow, selectedCol, 255, 0, 0); // Red
                            
                            // Show possible moves again
                            int moveCount = 0;
                            int moves[27][2];
                            _chessEngine->getPossibleMoves(board, selectedRow, selectedCol, moveCount, moves);
                            
                            for (int i = 0; i < moveCount; i++) {
                                _boardDriver->setSquareLED(moves[i][0], moves[i][1], 255, 255, 255); // White
                            }
                            _boardDriver->showLEDs();
                            
                            Serial.println("Piece is still selected. Place it on a valid move or return it to its original position.");
                        }
                        break;
                    }
                }
            }
        }
    }
    
    _boardDriver->updateSensorPrev();
}

bool ChessBot::connectToWiFi() {
    // Check for WiFi module
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("WiFi module not found!");
        return false;
    }
    
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        WiFi.begin(SECRET_SSID, SECRET_PASS);
        delay(5000);
        attempts++;
        
        Serial.print("Connection attempt ");
        Serial.print(attempts);
        Serial.print("/10 - Status: ");
        Serial.println(WiFi.status());
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("Failed to connect to WiFi");
        return false;
    }
}

String ChessBot::makeStockfishRequest(String fen) {
    WiFiSSLClient client;
    
    Serial.println("Making API request to Stockfish...");
    Serial.print("FEN: ");
    Serial.println(fen);
    
    if (client.connect(STOCKFISH_API_URL, STOCKFISH_API_PORT)) {
        // URL encode the FEN string
        String encodedFen = urlEncode(fen);
        
        // Make HTTP GET request
        String url = String(STOCKFISH_API_PATH) + "?fen=" + encodedFen + "&depth=" + String(settings.depth);
        
        Serial.print("Request URL: ");
        Serial.println(url);
        
        client.println("GET " + url + " HTTP/1.1");
        client.println("Host: " + String(STOCKFISH_API_URL));
        client.println("Connection: close");
        client.println();
        
        // Wait for response
        unsigned long startTime = millis();
        while (client.connected() && (millis() - startTime < settings.timeoutMs)) {
            if (client.available()) {
                String response = client.readString();
                client.stop();
                
                // Debug: Print raw response
                Serial.println("=== RAW API RESPONSE ===");
                Serial.println(response);
                Serial.println("=== END RAW RESPONSE ===");
                
                return response;
            }
            delay(10);
        }
        
        client.stop();
        Serial.println("API request timeout");
        return "";
    } else {
        Serial.println("Failed to connect to Stockfish API");
        return "";
    }
}

bool ChessBot::parseStockfishResponse(String response, String &bestMove) {
    // Find JSON content
    int jsonStart = response.indexOf("{");
    if (jsonStart == -1) {
        Serial.println("No JSON found in response");
        return false;
    }
    
    String json = response.substring(jsonStart);
    Serial.print("Extracted JSON: ");
    Serial.println(json);
    
    // Check if request was successful
    if (json.indexOf("\"success\":true") == -1) {
        Serial.println("API request was not successful");
        return false;
    }
    
    // Parse bestmove field - format: "bestmove":"bestmove b7b6 ponder f3e5"
    int bestmoveStart = json.indexOf("\"bestmove\":\"");
    if (bestmoveStart == -1) {
        Serial.println("No bestmove found in response");
        return false;
    }
    
    bestmoveStart += 12; // Skip "bestmove":"
    int bestmoveEnd = json.indexOf("\"", bestmoveStart);
    if (bestmoveEnd == -1) {
        Serial.println("Invalid bestmove format");
        return false;
    }
    
    String fullMove = json.substring(bestmoveStart, bestmoveEnd);
    Serial.print("Full move string: ");
    Serial.println(fullMove);
    
    // Extract just the move part after "bestmove " and before " ponder"
    int moveStart = fullMove.indexOf("bestmove ");
    if (moveStart == -1) {
        Serial.println("No 'bestmove' prefix found");
        return false;
    }
    
    moveStart += 9; // Skip "bestmove "
    int moveEnd = fullMove.indexOf(" ", moveStart);
    if (moveEnd == -1) {
        // No ponder part, take rest of string
        bestMove = fullMove.substring(moveStart);
    } else {
        bestMove = fullMove.substring(moveStart, moveEnd);
    }
    
    Serial.print("Parsed move: ");
    Serial.println(bestMove);
    
    return bestMove.length() >= 4;
}

void ChessBot::makeBotMove() {
    Serial.println("=== BOT MOVE CALCULATION ===");
    Serial.print("Bot is playing as: ");
    Serial.println(isWhiteTurn ? "White" : "Black");
    Serial.print("Current board state (FEN): ");
    
    String fen = boardToFEN();
    String response = makeStockfishRequest(fen);
    
    if (response.length() > 0) {
        String bestMove;
        if (parseStockfishResponse(response, bestMove)) {
            int fromRow, fromCol, toRow, toCol;
            if (parseMove(bestMove, fromRow, fromCol, toRow, toCol)) {
                Serial.print("Bot move: ");
                Serial.println(bestMove);
                
                executeBotMove(fromRow, fromCol, toRow, toCol);
                
                // Switch back to player's turn
                isWhiteTurn = true;
                botThinking = false;
                
                Serial.println("Bot move completed. Your turn!");
            } else {
                Serial.println("Failed to parse bot move");
                botThinking = false;
            }
        } else {
            Serial.println("Failed to parse Stockfish response");
            botThinking = false;
        }
    } else {
        Serial.println("No response from Stockfish API");
        botThinking = false;
    }
}

String ChessBot::boardToFEN() {
    String fen = "";
    
    // Board position - FEN expects rank 8 (black pieces) first, rank 1 (white pieces) last
    // Our array: row 0 = white pieces, row 7 = black pieces
    // So we need to reverse the order: start from row 7 and go to row 0
    for (int row = 7; row >= 0; row--) {
        int emptyCount = 0;
        for (int col = 0; col < 8; col++) {
            if (board[row][col] == ' ') {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen += String(emptyCount);
                    emptyCount = 0;
                }
                fen += board[row][col];
            }
        }
        if (emptyCount > 0) {
            fen += String(emptyCount);
        }
        if (row > 0) fen += "/";
    }
    
    // Active color - when we call this, it's the bot's turn (Black)
    // But we need to tell Stockfish whose turn it actually is
    fen += isWhiteTurn ? " w" : " b";
    
    Serial.print("Current turn in FEN: ");
    Serial.println(isWhiteTurn ? "White (w)" : "Black (b)");
    Serial.print("Bot should be playing as: Black");
    
    // Castling availability (simplified - assume all available initially)
    fen += " KQkq";
    
    // En passant target square (simplified - assume none)
    fen += " -";
    
    // Halfmove clock (simplified)
    fen += " 0";
    
    // Fullmove number (simplified)
    fen += " 1";
    
    Serial.print("Generated FEN: ");
    Serial.println(fen);
    
    return fen;
}

bool ChessBot::parseMove(String move, int &fromRow, int &fromCol, int &toRow, int &toCol) {
    if (move.length() < 4) return false;
    
    fromCol = move.charAt(0) - 'a';
    fromRow = (move.charAt(1) - '0') - 1;  // Convert 1-8 to 0-7 (1=row 0, 8=row 7)
    toCol = move.charAt(2) - 'a';
    toRow = (move.charAt(3) - '0') - 1;    // Convert 1-8 to 0-7
    
    // Debug coordinate conversion
    Serial.print("Move string: ");
    Serial.println(move);
    Serial.print("Parsed coordinates: (");
    Serial.print(fromRow);
    Serial.print(",");
    Serial.print(fromCol);
    Serial.print(") to (");
    Serial.print(toRow);
    Serial.print(",");
    Serial.print(toCol);
    Serial.println(")");
    Serial.print("In chess notation: ");
    Serial.print((char)('a' + fromCol));
    Serial.print(8 - fromRow);
    Serial.print(" to ");
    Serial.print((char)('a' + toCol));
    Serial.print(8 - toRow);
    
    // Check for promotion
    if (move.length() >= 5) {
        char promotionPiece = move.charAt(4);
        Serial.print(" (promotes to ");
        Serial.print(promotionPiece);
        Serial.print(")");
    }
    Serial.println();
    
    return (fromRow >= 0 && fromRow < 8 && fromCol >= 0 && fromCol < 8 &&
            toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8);
}

void ChessBot::executeBotMove(int fromRow, int fromCol, int toRow, int toCol) {
    char piece = board[fromRow][fromCol];
    char capturedPiece = board[toRow][toCol];
    
    // Update board state
    board[toRow][toCol] = piece;
    board[fromRow][fromCol] = ' ';
    
    Serial.print("Bot wants to move piece from ");
    Serial.print((char)('a' + fromCol));
    Serial.print(8 - fromRow);
    Serial.print(" to ");
    Serial.print((char)('a' + toCol));
    Serial.println(8 - toRow);
    Serial.println("Please make this move on the physical board...");
    
    // Show the move that needs to be made
    showBotMoveIndicator(fromRow, fromCol, toRow, toCol);
    
    // Wait for user to physically complete the bot's move
    waitForBotMoveCompletion(fromRow, fromCol, toRow, toCol);
    
    if (capturedPiece != ' ') {
        Serial.print("Piece captured: ");
        Serial.println(capturedPiece);
        _boardDriver->captureAnimation();
    }
    
    // Flash confirmation on the destination square
    confirmSquareCompletion(toRow, toCol);
    
    Serial.println("Bot move completed. Your turn!");
}

void ChessBot::showBotThinking() {
    static unsigned long lastUpdate = 0;
    static int thinkingStep = 0;
    
    if (millis() - lastUpdate > 500) {
        // Animated thinking indicator - pulse the corners
        _boardDriver->clearAllLEDs();
        
        uint8_t brightness = (sin(thinkingStep * 0.3) + 1) * 127;
        
        _boardDriver->setSquareLED(0, 0, 0, 0, brightness); // Corner LEDs pulse blue
        _boardDriver->setSquareLED(0, 7, 0, 0, brightness);
        _boardDriver->setSquareLED(7, 0, 0, 0, brightness);
        _boardDriver->setSquareLED(7, 7, 0, 0, brightness);
        
        _boardDriver->showLEDs();
        
        thinkingStep++;
        lastUpdate = millis();
    }
}

void ChessBot::showConnectionStatus() {
    // Show WiFi connection attempt with animated LEDs
    for (int i = 0; i < 8; i++) {
        _boardDriver->setSquareLED(3, i, 0, 0, 255); // Blue row
        _boardDriver->showLEDs();
        delay(200);
    }
}

void ChessBot::initializeBoard() {
    // Copy initial board state
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            board[row][col] = INITIAL_BOARD[row][col];
        }
    }
}

void ChessBot::waitForBoardSetup() {
    Serial.println("Please set up the chess board in starting position...");
    
    while (!_boardDriver->checkInitialBoard(INITIAL_BOARD)) {
        _boardDriver->readSensors();
        _boardDriver->updateSetupDisplay(INITIAL_BOARD);
        _boardDriver->showLEDs();
        delay(100);
    }
    
    Serial.println("Board setup complete! Game starting...");
    _boardDriver->fireworkAnimation();
    gameStarted = true;
    
    // Show initial board state
    printCurrentBoard();
}

void ChessBot::processPlayerMove(int fromRow, int fromCol, int toRow, int toCol, char piece) {
    char capturedPiece = board[toRow][toCol];
    
    // Update board state
    board[toRow][toCol] = piece;
    board[fromRow][fromCol] = ' ';
    
    Serial.print("Player moved ");
    Serial.print(piece);
    Serial.print(" from ");
    Serial.print((char)('a' + fromCol));
    Serial.print(8 - fromRow);
    Serial.print(" to ");
    Serial.print((char)('a' + toCol));
    Serial.println(8 - toRow);
    
    if (capturedPiece != ' ') {
        Serial.print("Captured ");
        Serial.println(capturedPiece);
        _boardDriver->captureAnimation();
    }
    
    // Check for pawn promotion
    if (_chessEngine->isPawnPromotion(piece, toRow)) {
        char promotedPiece = _chessEngine->getPromotedPiece(piece);
        board[toRow][toCol] = promotedPiece;
        Serial.print("Pawn promoted to ");
        Serial.println(promotedPiece);
        _boardDriver->promotionAnimation(toCol);
    }
}

String ChessBot::urlEncode(String str) {
    String encoded = "";
    char c;
    char code0;
    char code1;
    
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encoded += "%20";
        } else if (c == '/') {
            encoded += "%2F";
        } else if (isalnum(c)) {
            encoded += c;
        } else {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encoded += '%';
            encoded += code0;
            encoded += code1;
        }
    }
    return encoded;
}

void ChessBot::showBotMoveIndicator(int fromRow, int fromCol, int toRow, int toCol) {
    // Clear all LEDs first
    _boardDriver->clearAllLEDs();
    
    // Show source square flashing (where to pick up from)
    _boardDriver->setSquareLED(fromRow, fromCol, 255, 255, 255); // White flashing
    
    // Show destination square solid (where to place)
    _boardDriver->setSquareLED(toRow, toCol, 255, 255, 255);     // White solid
    
    _boardDriver->showLEDs();
}

void ChessBot::waitForBotMoveCompletion(int fromRow, int fromCol, int toRow, int toCol) {
    bool piecePickedUp = false;
    bool moveCompleted = false;
    static unsigned long lastBlink = 0;
    static bool blinkState = false;
    
    Serial.println("Waiting for you to complete the bot's move...");
    
    while (!moveCompleted) {
        _boardDriver->readSensors();
        
        // Blink the source square
        if (millis() - lastBlink > 500) {
            _boardDriver->clearAllLEDs();
            if (blinkState && !piecePickedUp) {
                _boardDriver->setSquareLED(fromRow, fromCol, 255, 255, 255); // Flash source
            }
            _boardDriver->setSquareLED(toRow, toCol, 255, 255, 255);         // Always show destination
            _boardDriver->showLEDs();
            
            blinkState = !blinkState;
            lastBlink = millis();
        }
        
        // Check if piece was picked up from source
        if (!piecePickedUp && !_boardDriver->getSensorState(fromRow, fromCol)) {
            piecePickedUp = true;
            Serial.println("Bot piece picked up, now place it on the destination...");
            
            // Stop blinking source, just show destination
            _boardDriver->clearAllLEDs();
            _boardDriver->setSquareLED(toRow, toCol, 255, 255, 255);
            _boardDriver->showLEDs();
        }
        
        // Check if piece was placed on destination
        if (piecePickedUp && _boardDriver->getSensorState(toRow, toCol)) {
            moveCompleted = true;
            Serial.println("Bot move completed on physical board!");
        }
        
        delay(50);
        _boardDriver->updateSensorPrev();
    }
}

void ChessBot::confirmMoveCompletion() {
    // This will be called with specific square coordinates when we need them
    confirmSquareCompletion(-1, -1); // Default - no specific square
}

void ChessBot::confirmSquareCompletion(int row, int col) {
    if (row >= 0 && col >= 0) {
        // Flash specific square twice
        for (int flash = 0; flash < 2; flash++) {
            _boardDriver->setSquareLED(row, col, 0, 255, 0); // Green flash
            _boardDriver->showLEDs();
            delay(150);
            
            _boardDriver->clearAllLEDs();
            _boardDriver->showLEDs();
            delay(150);
        }
    } else {
        // Flash entire board (fallback for when we don't have specific coords)
        for (int flash = 0; flash < 2; flash++) {
            for (int r = 0; r < 8; r++) {
                for (int c = 0; c < 8; c++) {
                    _boardDriver->setSquareLED(r, c, 0, 255, 0); // Green flash
                }
            }
            _boardDriver->showLEDs();
            delay(150);
            
            _boardDriver->clearAllLEDs();
            _boardDriver->showLEDs();
            delay(150);
        }
    }
}

void ChessBot::printCurrentBoard() {
    Serial.println("=== CURRENT BOARD STATE ===");
    Serial.println("  a b c d e f g h");
    for (int row = 0; row < 8; row++) {
        Serial.print(8 - row);
        Serial.print(" ");
        for (int col = 0; col < 8; col++) {
            char piece = board[row][col];
            if (piece == ' ') {
                Serial.print(". ");
            } else {
                Serial.print(piece);
                Serial.print(" ");
            }
        }
        Serial.print(" ");
        Serial.println(8 - row);
    }
    Serial.println("  a b c d e f g h");
    Serial.println("White pieces (uppercase): R N B Q K P");
    Serial.println("Black pieces (lowercase): r n b q k p");
    Serial.println("========================");
}

void ChessBot::setDifficulty(BotDifficulty diff) {
    difficulty = diff;
    switch(difficulty) {
        case BOT_EASY: settings = StockfishSettings::easy(); break;
        case BOT_MEDIUM: settings = StockfishSettings::medium(); break;
        case BOT_HARD: settings = StockfishSettings::hard(); break;
        case BOT_EXPERT: settings = StockfishSettings::expert(); break;
    }
    
    Serial.print("Bot difficulty changed to: ");
    switch(difficulty) {
        case BOT_EASY: Serial.println("Easy"); break;
        case BOT_MEDIUM: Serial.println("Medium"); break;
        case BOT_HARD: Serial.println("Hard"); break;
        case BOT_EXPERT: Serial.println("Expert"); break;
    }
}