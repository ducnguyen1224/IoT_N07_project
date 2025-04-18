#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "DHT.h"
#include <BH1750.h>
#include "config.h"

// Sensor data structure
struct SensorData {
    float temperature;
    float humidity;
    float lightLevel;
    int soilMoisture;
    int waterLevel;
    bool dhtValid;
    bool lightValid;
};


void setupSensors();


SensorData readSensors();

#endif // SENSORS_H