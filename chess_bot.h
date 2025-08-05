#ifndef CHESS_BOT_H
#define CHESS_BOT_H

#include "board_driver.h"
#include "chess_engine.h"
#include "stockfish_settings.h"
#include "arduino_secrets.h"
#include <WiFiNINA.h>
#include <WiFiSSLClient.h>

class ChessBot {
private:
    BoardDriver* _boardDriver;
    ChessEngine* _chessEngine;
    
    char board[8][8];
    const char INITIAL_BOARD[8][8] = {
        {'R','N','B','Q','K','B','N','R'},  // row 0 (rank 1)
        {'P','P','P','P','P','P','P','P'},  // row 1 (rank 2)
        {' ',' ',' ',' ',' ',' ',' ',' '},  // row 2 (rank 3)
        {' ',' ',' ',' ',' ',' ',' ',' '},  // row 3 (rank 4)
        {' ',' ',' ',' ',' ',' ',' ',' '},  // row 4 (rank 5)
        {' ',' ',' ',' ',' ',' ',' ',' '},  // row 5 (rank 6)
        {'p','p','p','p','p','p','p','p'},  // row 6 (rank 7)
        {'r','n','b','q','k','b','n','r'}   // row 7 (rank 8)
    };
    
    StockfishSettings settings;
    BotDifficulty difficulty;
    
    bool isWhiteTurn;
    bool gameStarted;
    bool botThinking;
    bool wifiConnected;
    
    // FEN notation handling
    String boardToFEN();
    void fenToBoard(String fen);
    
    // WiFi and API
    bool connectToWiFi();
    String makeStockfishRequest(String fen);
    bool parseStockfishResponse(String response, String &bestMove);
    
    // Move handling
    bool parseMove(String move, int &fromRow, int &fromCol, int &toRow, int &toCol);
    void executeBotMove(int fromRow, int fromCol, int toRow, int toCol);
    
    // URL encoding helper
    String urlEncode(String str);
    
    // Game flow
    void initializeBoard();
    void waitForBoardSetup();
    void processPlayerMove(int fromRow, int fromCol, int toRow, int toCol, char piece);
    void makeBotMove();
    void showBotThinking();
    void showConnectionStatus();
    void showBotMoveIndicator(int fromRow, int fromCol, int toRow, int toCol);
    void waitForBotMoveCompletion(int fromRow, int fromCol, int toRow, int toCol);
    void confirmMoveCompletion();
    void confirmSquareCompletion(int row, int col);
    void printCurrentBoard();
    
public:
    ChessBot(BoardDriver* boardDriver, ChessEngine* chessEngine, BotDifficulty diff = BOT_MEDIUM);
    void begin();
    void update();
    void setDifficulty(BotDifficulty diff);
};

#endif