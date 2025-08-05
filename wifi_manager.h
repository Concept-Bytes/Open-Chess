#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFiNINA.h>
#include <WiFiUdp.h>

// ---------------------------
// WiFi Configuration
// ---------------------------
#define AP_SSID "OpenChessBoard"
#define AP_PASSWORD "chess123"
#define AP_PORT 80

// ---------------------------
// WiFi Manager Class
// ---------------------------
class WiFiManager {
private:
    WiFiServer server;
    bool apMode;
    bool clientConnected;
    
    // Configuration variables
    String wifiSSID;
    String wifiPassword;
    String lichessToken;
    String gameMode;
    String startupType;
    
    // Web interface methods
    String generateWebPage();
    String generateGameSelectionPage();
    void handleConfigSubmit(WiFiClient& client, String request);
    void handleGameSelection(WiFiClient& client, String request);
    void sendResponse(WiFiClient& client, String content, String contentType = "text/html");
    void parseFormData(String data);
    
public:
    WiFiManager();
    void begin();
    void handleClient();
    bool isClientConnected();
    
    // Configuration getters
    String getWiFiSSID() { return wifiSSID; }
    String getWiFiPassword() { return wifiPassword; }
    String getLichessToken() { return lichessToken; }
    String getGameMode() { return gameMode; }
    String getStartupType() { return startupType; }
    
    // Game selection via web
    int getSelectedGameMode();
    void resetGameSelection();
};

#endif // WIFI_MANAGER_H