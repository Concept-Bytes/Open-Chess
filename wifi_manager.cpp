#include "wifi_manager.h"
#include <Arduino.h>

WiFiManager::WiFiManager() : server(AP_PORT) {
    apMode = true;
    clientConnected = false;
    wifiSSID = "";
    wifiPassword = "";
    lichessToken = "";
    gameMode = "None";
    startupType = "WiFi";
}

void WiFiManager::begin() {
    Serial.println("!!! WIFI MANAGER BEGIN FUNCTION CALLED !!!");
    Serial.println("=== Starting OpenChess WiFi Manager ===");
    Serial.println("Debug: WiFi Manager begin() called");
    
    // Check if WiFi is available
    Serial.println("Debug: Checking WiFi module...");
    
    // Try to get WiFi status - this often fails on incompatible boards
    Serial.println("Debug: Attempting to get WiFi status...");
    int initialStatus = WiFi.status();
    Serial.print("Debug: Initial WiFi status: ");
    Serial.println(initialStatus);
    
    // Initialize WiFi module
    Serial.println("Debug: Checking for WiFi module presence...");
    if (initialStatus == WL_NO_MODULE) {
        Serial.println("ERROR: WiFi module not detected!");
        Serial.println("Board type: Arduino Nano RP2040 - WiFi not supported with WiFiNINA");
        Serial.println("This is expected behavior for RP2040 boards.");
        Serial.println("Use physical board selectors for game mode selection.");
        return;
    }
    
    Serial.println("Debug: WiFi module appears to be present");
    
    Serial.println("Debug: WiFi module detected");
    
    // Check firmware version
    String fv = WiFi.firmwareVersion();
    Serial.print("Debug: WiFi firmware version: ");
    Serial.println(fv);
    
    // Start Access Point
    Serial.print("Debug: Creating Access Point with SSID: ");
    Serial.println(AP_SSID);
    Serial.print("Debug: Using password: ");
    Serial.println(AP_PASSWORD);
    
    Serial.println("Debug: Calling WiFi.beginAP()...");
    
    // First, try without channel specification (like Arduino example)
    Serial.println("Debug: Attempting AP creation without channel...");
    int status = WiFi.beginAP(AP_SSID, AP_PASSWORD);
    
    if (status != WL_AP_LISTENING) {
        Serial.println("Debug: First attempt failed, trying with channel 6...");
        status = WiFi.beginAP(AP_SSID, AP_PASSWORD, 6);
    }
    
    Serial.print("Debug: WiFi.beginAP() returned: ");
    Serial.println(status);
    
    // Print detailed status explanation
    Serial.print("Debug: Status meaning: ");
    switch(status) {
        case WL_IDLE_STATUS: Serial.println("WL_IDLE_STATUS (0) - Temporary status"); break;
        case WL_NO_SSID_AVAIL: Serial.println("WL_NO_SSID_AVAIL (1) - No SSID available"); break;
        case WL_SCAN_COMPLETED: Serial.println("WL_SCAN_COMPLETED (2) - Scan completed"); break;
        case WL_CONNECTED: Serial.println("WL_CONNECTED (3) - Connected to network"); break;
        case WL_CONNECT_FAILED: Serial.println("WL_CONNECT_FAILED (4) - Connection failed"); break;
        case WL_CONNECTION_LOST: Serial.println("WL_CONNECTION_LOST (5) - Connection lost"); break;
        case WL_DISCONNECTED: Serial.println("WL_DISCONNECTED (6) - Disconnected"); break;
        case WL_AP_LISTENING: Serial.println("WL_AP_LISTENING (7) - AP listening (SUCCESS!)"); break;
        case WL_AP_CONNECTED: Serial.println("WL_AP_CONNECTED (8) - AP connected"); break;
        case WL_AP_FAILED: Serial.println("WL_AP_FAILED (9) - AP failed"); break;
        default: Serial.print("UNKNOWN STATUS ("); Serial.print(status); Serial.println(")"); break;
    }
    
    if (status != WL_AP_LISTENING) {
        Serial.println("ERROR: Failed to create Access Point!");
        Serial.println("Expected WL_AP_LISTENING (7), but got different status");
        return;
    }
    
    Serial.println("Debug: Access Point creation initiated");
    
    // Wait for AP to start and check status
    Serial.println("Debug: Waiting for AP to start...");
    for (int i = 0; i < 10; i++) {
        delay(1000);
        status = WiFi.status();
        Serial.print("Debug: WiFi status check ");
        Serial.print(i + 1);
        Serial.print("/10 - Status: ");
        Serial.println(status);
        
        if (status == WL_AP_LISTENING) {
            Serial.println("Debug: AP is now listening!");
            break;
        }
    }
    
    // Print AP information and verify it's actually working
    IPAddress ip = WiFi.localIP();
    Serial.println("=== WiFi Access Point Information ===");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());  // Get actual SSID from WiFi module
    Serial.print("Expected SSID: ");
    Serial.println(AP_SSID);
    Serial.print("Password: ");
    Serial.println(AP_PASSWORD);
    Serial.print("IP Address: ");
    Serial.println(ip);
    Serial.print("Web Interface: http://");
    Serial.println(ip);
    
    // Additional diagnostic information
    Serial.print("WiFi Status: ");
    Serial.println(WiFi.status());
    Serial.print("WiFi Mode: ");
    // Note: WiFiNINA might not have explicit AP mode check
    Serial.println("Access Point Mode");
    
    // Verify IP is valid
    if (ip == IPAddress(0, 0, 0, 0)) {
        Serial.println("WARNING: IP address is 0.0.0.0 - AP might not be working!");
    } else {
        Serial.println("IP address looks valid");
    }
    
    Serial.println("=====================================");
    
    // Start the web server
    Serial.println("Debug: Starting web server...");
    server.begin();
    Serial.println("Debug: Web server started on port 80");
    Serial.println("WiFi Manager initialization complete!");
}

void WiFiManager::handleClient() {
    WiFiClient client = server.available();
    
    if (client) {
        clientConnected = true;
        Serial.println("New client connected");
        
        String request = "";
        bool currentLineIsBlank = true;
        bool readingBody = false;
        String body = "";
        
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                
                if (!readingBody) {
                    request += c;
                    
                    if (c == '\n' && currentLineIsBlank) {
                        // Headers ended, now reading body if POST
                        if (request.indexOf("POST") >= 0) {
                            readingBody = true;
                        } else {
                            break; // GET request, no body
                        }
                    }
                    
                    if (c == '\n') {
                        currentLineIsBlank = true;
                    } else if (c != '\r') {
                        currentLineIsBlank = false;
                    }
                } else {
                    // Reading POST body
                    body += c;
                    if (body.length() > 1000) break; // Prevent overflow
                }
            }
        }
        
        // Handle the request
        if (request.indexOf("GET / ") >= 0) {
            // Main configuration page
            String webpage = generateWebPage();
            sendResponse(client, webpage);
        }
        else if (request.indexOf("GET /game") >= 0) {
            // Game selection page
            String gameSelectionPage = generateGameSelectionPage();
            sendResponse(client, gameSelectionPage);
        }
        else if (request.indexOf("POST /submit") >= 0) {
            // Configuration form submission
            parseFormData(body);
            String response = "<html><body style='font-family:Arial;background:#5c5d5e;color:#ec8703;text-align:center;padding:50px;'>";
            response += "<h2>Configuration Saved!</h2>";
            response += "<p>WiFi SSID: " + wifiSSID + "</p>";
            response += "<p>Game Mode: " + gameMode + "</p>";
            response += "<p>Startup Type: " + startupType + "</p>";
            response += "<p><a href='/game' style='color:#ec8703;'>Go to Game Selection</a></p>";
            response += "</body></html>";
            sendResponse(client, response);
        }
        else if (request.indexOf("POST /gameselect") >= 0) {
            // Game selection submission
            handleGameSelection(client, body);
        }
        else {
            // 404 Not Found
            String response = "<html><body style='font-family:Arial;background:#5c5d5e;color:#ec8703;text-align:center;padding:50px;'>";
            response += "<h2>404 - Page Not Found</h2>";
            response += "<p><a href='/' style='color:#ec8703;'>Back to Home</a></p>";
            response += "</body></html>";
            sendResponse(client, response, "text/html");
        }
        
        delay(10);
        client.stop();
        Serial.println("Client disconnected");
        clientConnected = false;
    }
}

String WiFiManager::generateWebPage() {
    String html = "<!DOCTYPE html>";
    html += "<html lang=\"en\">";
    html += "<head>";
    html += "<meta charset=\"UTF-8\">";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    html += "<title>OPENCHESSBOARD CONFIGURATION</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; background-color: #5c5d5e; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; min-height: 100vh; }";
    html += ".container { background-color: #353434; border-radius: 8px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); padding: 30px; width: 100%; max-width: 500px; }";
    html += "h2 { text-align: center; color: #ec8703; font-size: 24px; margin-bottom: 20px; }";
    html += "label { font-size: 16px; color: #ec8703; margin-bottom: 8px; display: block; }";
    html += "input[type=\"text\"], input[type=\"password\"], select { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; box-sizing: border-box; font-size: 16px; }";
    html += "input[type=\"submit\"], .button { background-color: #ec8703; color: white; border: none; padding: 15px; font-size: 16px; width: 100%; border-radius: 5px; cursor: pointer; transition: background-color 0.3s ease; text-decoration: none; display: block; text-align: center; margin: 10px 0; }";
    html += "input[type=\"submit\"]:hover, .button:hover { background-color: #ebca13; }";
    html += ".form-group { margin-bottom: 15px; }";
    html += ".note { font-size: 14px; color: #ec8703; text-align: center; margin-top: 20px; }";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<div class=\"container\">";
    html += "<h2>OPENCHESSBOARD CONFIGURATION</h2>";
    html += "<form action=\"/submit\" method=\"POST\">";
    
    html += "<div class=\"form-group\">";
    html += "<label for=\"ssid\">WiFi SSID:</label>";
    html += "<input type=\"text\" name=\"ssid\" id=\"ssid\" value=\"" + wifiSSID + "\" placeholder=\"Enter Your WiFi SSID\">";
    html += "</div>";
    
    html += "<div class=\"form-group\">";
    html += "<label for=\"password\">WiFi Password:</label>";
    html += "<input type=\"password\" name=\"password\" id=\"password\" value=\"\" placeholder=\"Enter Your WiFi Password\">";
    html += "</div>";
    
    html += "<div class=\"form-group\">";
    html += "<label for=\"token\">Lichess Token (Optional):</label>";
    html += "<input type=\"text\" name=\"token\" id=\"token\" value=\"" + lichessToken + "\" placeholder=\"Enter Your Lichess Token (Future Feature)\">";
    html += "</div>";
    
    html += "<div class=\"form-group\">";
    html += "<label for=\"gameMode\">Default Game Mode:</label>";
    html += "<select name=\"gameMode\" id=\"gameMode\">";
    html += "<option value=\"None\"";
    if (gameMode == "None") html += " selected";
    html += ">Local Chess Only</option>";
    html += "<option value=\"5+3\"";
    if (gameMode == "5+3") html += " selected";
    html += ">5+3 (Future)</option>";
    html += "<option value=\"10+5\"";
    if (gameMode == "10+5") html += " selected";
    html += ">10+5 (Future)</option>";
    html += "<option value=\"15+10\"";
    if (gameMode == "15+10") html += " selected";
    html += ">15+10 (Future)</option>";
    html += "<option value=\"AI level 1\"";
    if (gameMode == "AI level 1") html += " selected";
    html += ">AI level 1 (Future)</option>";
    html += "<option value=\"AI level 2\"";
    if (gameMode == "AI level 2") html += " selected";
    html += ">AI level 2 (Future)</option>";
    html += "</select>";
    html += "</div>";
    
    html += "<div class=\"form-group\">";
    html += "<label for=\"startupType\">Default Startup Type:</label>";
    html += "<select name=\"startupType\" id=\"startupType\">";
    html += "<option value=\"WiFi\"";
    if (startupType == "WiFi") html += " selected";
    html += ">WiFi Mode</option>";
    html += "<option value=\"Local\"";
    if (startupType == "Local") html += " selected";
    html += ">Local Mode</option>";
    html += "</select>";
    html += "</div>";
    
    html += "<input type=\"submit\" value=\"Save Configuration\">";
    html += "</form>";
    html += "<a href=\"/game\" class=\"button\">Game Selection Interface</a>";
    html += "<div class=\"note\">";
    html += "<p>Configure your OpenChess board settings and WiFi connection.</p>";
    html += "</div>";
    html += "</div>";
    html += "</body>";
    html += "</html>";
    
    return html;
}

String WiFiManager::generateGameSelectionPage() {
    String html = "<!DOCTYPE html>";
    html += "<html lang=\"en\">";
    html += "<head>";
    html += "<meta charset=\"UTF-8\">";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    html += "<title>OPENCHESSBOARD GAME SELECTION</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; background-color: #5c5d5e; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; min-height: 100vh; }";
    html += ".container { background-color: #353434; border-radius: 8px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); padding: 30px; width: 100%; max-width: 600px; }";
    html += "h2 { text-align: center; color: #ec8703; font-size: 24px; margin-bottom: 30px; }";
    html += ".game-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; margin-bottom: 30px; }";
    html += ".game-mode { background-color: #444; border: 2px solid #ec8703; border-radius: 8px; padding: 20px; text-align: center; cursor: pointer; transition: all 0.3s ease; color: #fff; }";
    html += ".game-mode:hover { background-color: #ec8703; transform: translateY(-2px); }";
    html += ".game-mode.available { border-color: #4CAF50; }";
    html += ".game-mode.coming-soon { border-color: #888; opacity: 0.6; }";
    html += ".game-mode h3 { margin: 0 0 10px 0; font-size: 18px; }";
    html += ".game-mode p { margin: 0; font-size: 14px; opacity: 0.8; }";
    html += ".status { font-size: 12px; padding: 5px 10px; border-radius: 15px; margin-top: 10px; display: inline-block; }";
    html += ".available .status { background-color: #4CAF50; color: white; }";
    html += ".coming-soon .status { background-color: #888; color: white; }";
    html += ".back-button { background-color: #666; color: white; border: none; padding: 15px; font-size: 16px; width: 100%; border-radius: 5px; cursor: pointer; text-decoration: none; display: block; text-align: center; margin-top: 20px; }";
    html += ".back-button:hover { background-color: #777; }";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<div class=\"container\">";
    html += "<h2>GAME SELECTION</h2>";
    html += "<div class=\"game-grid\">";
    
    html += "<div class=\"game-mode available\" onclick=\"selectGame(1)\">";
    html += "<h3>Chess Moves</h3>";
    html += "<p>Full chess game with move validation and animations</p>";
    html += "<span class=\"status\">Available</span>";
    html += "</div>";
    
    html += "<div class=\"game-mode coming-soon\">";
    html += "<h3>Game Mode 2</h3>";
    html += "<p>Future game mode placeholder</p>";
    html += "<span class=\"status\">Coming Soon</span>";
    html += "</div>";
    
    html += "<div class=\"game-mode coming-soon\">";
    html += "<h3>Game Mode 3</h3>";
    html += "<p>Future game mode placeholder</p>";
    html += "<span class=\"status\">Coming Soon</span>";
    html += "</div>";
    
    html += "<div class=\"game-mode available\" onclick=\"selectGame(4)\">";
    html += "<h3>Sensor Test</h3>";
    html += "<p>Test and calibrate board sensors</p>";
    html += "<span class=\"status\">Available</span>";
    html += "</div>";
    
    html += "</div>";
    html += "<a href=\"/\" class=\"back-button\">Back to Configuration</a>";
    html += "</div>";
    
    html += "<script>";
    html += "function selectGame(mode) {";
    html += "if (mode === 1 || mode === 4) {";
    html += "fetch('/gameselect', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: 'gamemode=' + mode })";
    html += ".then(response => response.text())";
    html += ".then(data => { alert('Game mode ' + mode + ' selected! Check your chess board.'); })";
    html += ".catch(error => { console.error('Error:', error); });";
    html += "} else { alert('This game mode is coming soon!'); }";
    html += "}";
    html += "</script>";
    html += "</body>";
    html += "</html>";
    
    return html;
}

void WiFiManager::handleGameSelection(WiFiClient& client, String body) {
    // Parse game mode selection
    int modeStart = body.indexOf("gamemode=");
    if (modeStart >= 0) {
        int modeEnd = body.indexOf("&", modeStart);
        if (modeEnd < 0) modeEnd = body.length();
        
        String selectedMode = body.substring(modeStart + 9, modeEnd);
        int mode = selectedMode.toInt();
        
        Serial.print("Game mode selected via web: ");
        Serial.println(mode);
        
        // Store the selected game mode (you'll access this from main code)
        gameMode = String(mode);
        
        String response = R"({"status":"success","message":"Game mode selected","mode":)" + String(mode) + "}";
        sendResponse(client, response, "application/json");
    }
}

void WiFiManager::sendResponse(WiFiClient& client, String content, String contentType) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: " + contentType);
    client.println("Connection: close");
    client.println();
    client.println(content);
}

void WiFiManager::parseFormData(String data) {
    // Parse URL-encoded form data
    int ssidStart = data.indexOf("ssid=");
    if (ssidStart >= 0) {
        int ssidEnd = data.indexOf("&", ssidStart);
        if (ssidEnd < 0) ssidEnd = data.length();
        wifiSSID = data.substring(ssidStart + 5, ssidEnd);
        wifiSSID.replace("+", " ");
    }
    
    int passStart = data.indexOf("password=");
    if (passStart >= 0) {
        int passEnd = data.indexOf("&", passStart);
        if (passEnd < 0) passEnd = data.length();
        wifiPassword = data.substring(passStart + 9, passEnd);
    }
    
    int tokenStart = data.indexOf("token=");
    if (tokenStart >= 0) {
        int tokenEnd = data.indexOf("&", tokenStart);
        if (tokenEnd < 0) tokenEnd = data.length();
        lichessToken = data.substring(tokenStart + 6, tokenEnd);
    }
    
    int gameModeStart = data.indexOf("gameMode=");
    if (gameModeStart >= 0) {
        int gameModeEnd = data.indexOf("&", gameModeStart);
        if (gameModeEnd < 0) gameModeEnd = data.length();
        gameMode = data.substring(gameModeStart + 9, gameModeEnd);
        gameMode.replace("+", " ");
    }
    
    int startupStart = data.indexOf("startupType=");
    if (startupStart >= 0) {
        int startupEnd = data.indexOf("&", startupStart);
        if (startupEnd < 0) startupEnd = data.length();
        startupType = data.substring(startupStart + 12, startupEnd);
    }
    
    Serial.println("Configuration updated:");
    Serial.println("SSID: " + wifiSSID);
    Serial.println("Game Mode: " + gameMode);
    Serial.println("Startup Type: " + startupType);
}

bool WiFiManager::isClientConnected() {
    return clientConnected;
}

int WiFiManager::getSelectedGameMode() {
    return gameMode.toInt();
}

void WiFiManager::resetGameSelection() {
    gameMode = "0";
}