#ifndef CHESS_MOVES_H
#define CHESS_MOVES_H

#include "board_driver.h"
#include "chess_engine.h"

// ---------------------------
// Chess Game Mode Class
// ---------------------------
class ChessMoves {
private:
    BoardDriver* boardDriver;
    ChessEngine* chessEngine;
    
    // Expected initial configuration
    static const char INITIAL_BOARD[8][8];
    
    // Internal board state for gameplay
    char board[8][8];
    
    // Helper functions
    void initializeBoard();
    void waitForBoardSetup();
    void processMove(int fromRow, int fromCol, int toRow, int toCol, char piece);
    void checkForPromotion(int targetRow, int targetCol, char piece);
    void handlePromotion(int targetRow, int targetCol, char piece);

public:
    ChessMoves(BoardDriver* bd, ChessEngine* ce);
    void begin();
    void update();
    bool isActive();
    void reset();
};

#endif // CHESS_MOVES_H