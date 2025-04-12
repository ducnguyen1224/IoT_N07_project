#include <Arduino.h>
#include <Wire.h>
#include "DHT.h"
#include <BH1750.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>  // Thêm thư viện cho kết nối SSL

// Định nghĩa các chân và thông số
#define DHTPIN 4        // Chân kết nối DHT11
#define DHTTYPE DHT11   // Loại cảm biến DHT
#define SOIL_SENSOR_PIN 32  // Chân GPIO32 trên ESP32 dùng cho cảm biến độ ẩm đất
#define WATER_LEVEL_PIN 33  // Chân GPIO33 trên ESP32 dùng cho cảm biến mực nước

// Thông tin WiFi
const char* ssid = "PTIT_WIFI";
const char* password = "";

// Thông tin HiveMQ Cloud MQTT Broker
const char* mqtt_server = "9f2157c861a94427a60f119c9f7370e2.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  // Dùng TLS/SSL (port 8883)
const char* mqtt_user = "hivemq.webclient.1742874741899";
const char* mqtt_password = "eV7d41D0NG.g:X>icz#H";

// Tạo client WiFi Secure
WiFiClientSecure espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

unsigned long lastMsg = 0;  // Biến lưu thời gian gửi tin nhắn cuối cùng

// Hàm kết nối WiFi
void setup_wifi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected!");
    
    // Hiển thị thông tin WiFi đã kết nối
    Serial.println("Connected to WiFi network:");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Cấu hình SSL cho MQTT
    espClient.setInsecure(); // Bỏ qua xác thực chứng chỉ (không nên dùng trong production)
    // Hoặc có thể set CA Certificate nếu có:
    // espClient.setCACert(ca_cert);
}

// Hàm callback khi nhận được tin nhắn MQTT
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

// Hàm kết nối lại MQTT nếu mất kết nối
void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection to HiveMQ Cloud...");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
            Serial.println("connected to HiveMQ Cloud!");
            client.publish("esp32/status", "ESP32 connected to HiveMQ");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println(F("Initializing sensors..."));
    
    dht.begin();
    Wire.begin();
    lightMeter.begin();
    pinMode(SOIL_SENSOR_PIN, INPUT);
    pinMode(WATER_LEVEL_PIN, INPUT);  // Khởi tạo chân cảm biến mực nước
    
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    
    Serial.println(F("Sensors initialized successfully"));
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    
    unsigned long now = millis();
    if (now - lastMsg > 2000) {  // Gửi dữ liệu mỗi 2 giây
        lastMsg = now;
        
        // Đọc dữ liệu từ cảm biến DHT11
        float humidity = dht.readHumidity();
        float tempC = dht.readTemperature();
        
        if (!isnan(humidity) && !isnan(tempC)) {
            Serial.print(F("Humidity: "));
            Serial.print(humidity);
            Serial.print(F("%  Temperature: "));
            Serial.print(tempC);
            Serial.println(F("°C"));
            
            client.publish("esp32/sensor/temperature", String(tempC).c_str());
            client.publish("esp32/sensor/humidity", String(humidity).c_str());
        } else {
            Serial.println(F("Failed to read from DHT sensor!"));
        }
        
        // Đọc dữ liệu từ cảm biến ánh sáng BH1750
        float lux = lightMeter.readLightLevel();
        Serial.print(F("Light Intensity: "));
        Serial.print(lux);
        Serial.println(F(" lx"));
        client.publish("esp32/sensor/light", String(lux).c_str());
        
        // Đọc dữ liệu từ cảm biến độ ẩm đất
        int soilMoisture = analogRead(SOIL_SENSOR_PIN);
        Serial.print(F("Soil Moisture Level: "));
        Serial.println(soilMoisture);
        client.publish("esp32/sensor/soil", String(soilMoisture).c_str());
        
        // Đọc dữ liệu từ cảm biến mực nước
        int waterLevel = analogRead(WATER_LEVEL_PIN);
        Serial.print(F("Water Level: "));
        Serial.println(waterLevel);
        client.publish("esp32/sensor/water_level", String(waterLevel).c_str());
    }
}