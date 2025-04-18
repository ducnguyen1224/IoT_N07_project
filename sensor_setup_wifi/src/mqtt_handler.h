#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "config.h"
#include "sensors.h"

// Setup MQTT client
void setupMQTT();

// Connect to MQTT broker
bool connectMQTT();

// Publish sensor data to MQTT
void publishSensorData(const SensorData& data);

// Check MQTT connection and reconnect if needed
bool checkMQTTConnection();

// Get MQTT connection status
bool isMQTTConnected();

// Get reference to the MQTT client object
PubSubClient& getMQTTClient();

#endif // MQTT_HANDLER_H