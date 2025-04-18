#include "sensors.h"
#include <Wire.h>

// Global sensor objects
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

void setupSensors() {
    
    dht.begin();
    
    
    Wire.begin();
    if (lightMeter.begin()) {
        Serial.println("BH1750 light sensor initialized");
    } else {
        Serial.println("Error initializing BH1750 light sensor!");
    }
    
    // Set analog pins for soil moisture and water level
    pinMode(SOIL_SENSOR_PIN, INPUT);
    pinMode(WATER_LEVEL_PIN, INPUT);
    
    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("All sensors initialized");
}

SensorData readSensors() {
    SensorData data;
    
    // Read DHT11 sensor
    data.humidity = dht.readHumidity();
    data.temperature = dht.readTemperature();
    
    if (isnan(data.humidity) || isnan(data.temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        data.dhtValid = false;
    } else {
        data.dhtValid = true;
        Serial.print("Humidity: ");
        Serial.print(data.humidity);
        Serial.print("%  Temperature: ");
        Serial.print(data.temperature);
        Serial.println("°C");
    }
    
    // Read BH1750 light sensor
    data.lightLevel = lightMeter.readLightLevel();
    data.lightValid = !isnan(data.lightLevel);
    if (data.lightValid) {
        Serial.print("Light Intensity: ");
        Serial.print(data.lightLevel);
        Serial.println(" lx");
    } else {
        Serial.println("Failed to read from light sensor!");
    }
    
    // Read soil moisture sensor
    data.soilMoisture = analogRead(SOIL_SENSOR_PIN);
    Serial.print("Soil Moisture Level: ");
    Serial.println(data.soilMoisture);
    
    // Read water level sensor
    data.waterLevel = analogRead(WATER_LEVEL_PIN);
    Serial.print("Water Level: ");
    Serial.println(data.waterLevel);
    
    return data;
}