#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Sensor pins and types
#define DHTPIN 4        // DHT11 connection pin
#define DHTTYPE DHT11   // Type of DHT sensor
#define SOIL_SENSOR_PIN 32  // Soil moisture sensor pin
#define WATER_LEVEL_PIN 33  // Water level sensor pin
#define LED_PIN 2       // Built-in LED

// MQTT Broker Information
const char* const MQTT_SERVER = "9f2157c861a94427a60f119c9f7370e2.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;  // TLS/SSL port
const char* const MQTT_USER = "hivemq.webclient.1742874741899";
const char* const MQTT_PASSWORD = "eV7d41D0NG.g:X>icz#H";

// WiFi Manager settings
const char* const AP_NAME = "ESP32-Garden-Setup";
const char* const AP_PASSWORD = "password123";
const char* const DEVICE_HOSTNAME = "ESP32-Garden";

// MQTT Topics
const char* const TOPIC_TEMPERATURE = "esp32/sensor/temperature";
const char* const TOPIC_HUMIDITY = "esp32/sensor/humidity";
const char* const TOPIC_LIGHT = "esp32/sensor/light";
const char* const TOPIC_SOIL = "esp32/sensor/soil";
const char* const TOPIC_WATER = "esp32/sensor/water_level";
const char* const TOPIC_STATUS = "esp32/status";

// Global timing variables
const unsigned long SENSOR_READ_INTERVAL = 2000;  // 2 seconds
const unsigned long WEB_UPDATE_INTERVAL = 5000;   // 5 seconds

#endif // CONFIG_H