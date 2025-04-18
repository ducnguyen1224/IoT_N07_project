#include "mqtt_handler.h"
#include "wifi_manager.h"  // Add this to access blinkLED function

// Global MQTT objects
WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// MQTT message callback
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void setupMQTT() {
    // Configure SSL for MQTT
    espClient.setInsecure(); // Skip certificate validation
    
    // Setup MQTT client
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(callback);
    
    Serial.println("MQTT client configured");
}

bool connectMQTT() {
    int attempts = 0;
    while (!mqttClient.connected() && attempts < 5) {
        Serial.print("Attempting MQTT connection to HiveMQ Cloud...");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        
        if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
            Serial.println("connected to HiveMQ Cloud!");
            mqttClient.publish(TOPIC_STATUS, "ESP32 connected to HiveMQ");
            blinkLED(2); // Blink twice to indicate MQTT connection
            return true;
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            blinkLED(1);
            delay(5000);
            attempts++;
        }
    }
    return false;
}

void publishSensorData(const SensorData& data) {
    if (mqttClient.connected()) {
        // Only publish valid DHT data
        if (data.dhtValid) {
            mqttClient.publish(TOPIC_TEMPERATURE, String(data.temperature).c_str());
            mqttClient.publish(TOPIC_HUMIDITY, String(data.humidity).c_str());
        }
        
        // Only publish valid light data
        if (data.lightValid) {
            mqttClient.publish(TOPIC_LIGHT, String(data.lightLevel).c_str());
        }
        
        // Always publish analog values
        mqttClient.publish(TOPIC_SOIL, String(data.soilMoisture).c_str());
        mqttClient.publish(TOPIC_WATER, String(data.waterLevel).c_str());
        
        // Blink LED briefly to indicate successful publish
        digitalWrite(LED_PIN, HIGH);
        delay(50);
        digitalWrite(LED_PIN, LOW);
    }
}

bool checkMQTTConnection() {
    if (!mqttClient.connected()) {
        return connectMQTT();
    }
    return true;
}

bool isMQTTConnected() {
    return mqttClient.connected();
}

// Make the PubSubClient object accessible to other modules
PubSubClient& getMQTTClient() {
    return mqttClient;
}