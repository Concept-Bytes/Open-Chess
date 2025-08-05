#ifndef WIFI_MANAGER_RP2040_H
#define WIFI_MANAGER_RP2040_H

// Simple WiFi Manager for RP2040 using built-in WiFi
// This is a simplified version that doesn't require external WiFi modules

#include <Arduino.h>

// ---------------------------
// WiFi Manager Class for RP2040
// ---------------------------
class WiFiManagerRP2040 {
private:
    bool wifiEnabled;
    String gameMode;
    
public:
    WiFiManagerRP2040();
    void begin();
    void handleClient();
    bool isClientConnected();
    
    // Configuration getters
    String getWiFiSSID() { return ""; }
    String getWiFiPassword() { return ""; }
    String getLichessToken() { return ""; }
    String getGameMode() { return gameMode; }
    String getStartupType() { return "Local"; }
    
    // Game selection via web
    int getSelectedGameMode();
    void resetGameSelection();
};

#endif // WIFI_MANAGER_RP2040_H