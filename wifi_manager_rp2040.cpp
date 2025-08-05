#include "wifi_manager_rp2040.h"

WiFiManagerRP2040::WiFiManagerRP2040() {
    wifiEnabled = false;
    gameMode = "0";
}

void WiFiManagerRP2040::begin() {
    Serial.println("WiFi Manager for RP2040 - Placeholder Implementation");
    Serial.println("Full WiFi features require external WiFi module");
    Serial.println("Use physical board selectors for game mode selection");
    wifiEnabled = false;
}

void WiFiManagerRP2040::handleClient() {
    // Placeholder - no WiFi functionality
    // This prevents compilation errors when WiFi is enabled
    // but provides no actual WiFi features
}

bool WiFiManagerRP2040::isClientConnected() {
    return false;
}

int WiFiManagerRP2040::getSelectedGameMode() {
    return 0; // No game mode selected via WiFi
}

void WiFiManagerRP2040::resetGameSelection() {
    gameMode = "0";
}