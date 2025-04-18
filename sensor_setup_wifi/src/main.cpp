#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "wifi_manager.h"
#include "mqtt_handler.h"
#include "web_server.h"

// Timing variables
unsigned long lastSensorReadTime = 0;
unsigned long lastWebUpdateTime = 0;

// Sensor data
SensorData sensorData;

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.println("\nStarting ESP32 Garden Monitoring System...");
    
    // Initialize sensors
    setupSensors();
    
    // Initialize WiFi
    if (setupWiFi()) {
        // Initialize MQTT
        setupMQTT();
        connectMQTT();
        
        // Initialize web server
        setupWebServer();
    } else {
        Serial.println("WiFi setup failed. Restarting...");
        delay(3000);
        ESP.restart();
    }
    
    Serial.println("Setup complete!");
}

void loop() {
    // Handle web server client requests
    handleWebClientRequests();
    
    // Check and maintain MQTT connection
    if (!checkMQTTConnection()) {
        Serial.println("MQTT connection failed. Will retry...");
    }
    

    getMQTTClient().loop();
    
    // Read sensors and publish data periodically
    unsigned long currentTime = millis();
    
    // Read sensors at specified interval
    if (currentTime - lastSensorReadTime > SENSOR_READ_INTERVAL) {
        lastSensorReadTime = currentTime;
        
        // Read sensor data
        sensorData = readSensors();
        
        // Publish to MQTT
        publishSensorData(sensorData);
    }
    
    // Update web server data at specified interval
    if (currentTime - lastWebUpdateTime > WEB_UPDATE_INTERVAL) {
        lastWebUpdateTime = currentTime;
        updateWebServerData(sensorData);
    }
}