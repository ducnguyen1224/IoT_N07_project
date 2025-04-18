#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFiManager.h>
#include "config.h"

// Functions for WiFi management
bool setupWiFi();
void blinkLED(int times);

#endif // WIFI_MANAGER_H