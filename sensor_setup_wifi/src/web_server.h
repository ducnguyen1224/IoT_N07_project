#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include "config.h"
#include "sensors.h"
#include "mqtt_handler.h"

// Initialize the web server
void setupWebServer();

// Update web server with latest sensor data
void updateWebServerData(const SensorData& data);

// Process web server client requests
void handleWebClientRequests();

#endif // WEB_SERVER_H