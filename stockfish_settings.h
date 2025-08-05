#ifndef STOCKFISH_SETTINGS_H
#define STOCKFISH_SETTINGS_H

// Stockfish Engine Settings
struct StockfishSettings {
    int depth = 12;                    // Search depth (1-15, higher = stronger but slower)
    int timeoutMs = 30000;             // API timeout in milliseconds (30 seconds)
    bool useBook = true;               // Use opening book for first moves
    int maxRetries = 3;                // Max API call retries on failure
    
    // Difficulty presets
    static StockfishSettings easy() {
        StockfishSettings s;
        s.depth = 6;
        s.timeoutMs = 15000;
        return s;
    }
    
    static StockfishSettings medium() {
        StockfishSettings s;
        s.depth = 6;
        s.timeoutMs = 25000;
        return s;
    }
    
    static StockfishSettings hard() {
        StockfishSettings s;
        s.depth = 14;
        s.timeoutMs = 45000;
        return s;
    }
    
    static StockfishSettings expert() {
        StockfishSettings s;
        s.depth = 16;
        s.timeoutMs = 60000;
        return s;
    }
};

// Bot difficulty levels
enum BotDifficulty {
    BOT_EASY = 1,
    BOT_MEDIUM = 2,
    BOT_HARD = 3,
    BOT_EXPERT = 4
};

#endif