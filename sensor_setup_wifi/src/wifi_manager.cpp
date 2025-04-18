#include "wifi_manager.h"

bool setupWiFi() {
    // WiFiManager instance
    WiFiManager wm;
    
    // Uncomment to reset saved settings
    // wm.resetSettings();
    
    // Set config portal timeout (seconds)
    wm.setConfigPortalTimeout(180);
    
    // Set custom hostname
    wm.setHostname(DEVICE_HOSTNAME);
    
    // Set custom AP name and password
    bool res = wm.autoConnect(AP_NAME, AP_PASSWORD);
    
    if(!res) {
        Serial.println("Failed to connect to WiFi");
        // Blink LED to indicate failure
        blinkLED(5);
        return false;
    } else {
        Serial.println("Connected to WiFi successfully");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
        // Blink LED once to indicate success
        blinkLED(1);
        return true;
    }
}

void blinkLED(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
}