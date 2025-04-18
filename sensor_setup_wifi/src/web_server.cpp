#include "web_server.h"

// Global WebServer object
WebServer server(80);

// Global sensor data for web display
SensorData latestData;

// Handler functions
void handleRoot();
void handleMQTTConfig();
void handleNotFound();
void handleResetWiFi();  

void setupWebServer() {
    // Set up mDNS responder
    if (MDNS.begin("esp32garden")) {
        Serial.println("MDNS responder started - access at http://esp32garden.local");
    }
    
    // Define server routes
    server.on("/", handleRoot);
    server.on("/mqtt-config", handleMQTTConfig);
    
    // 404 handler
    server.onNotFound(handleNotFound);
    
    server.on("/reset", handleResetWiFi);// Reset WiFi settings

    // Start server
    server.begin();
    Serial.println("HTTP server started");
}

void updateWebServerData(const SensorData& data) {
    // Update global copy of sensor data for web display
    latestData = data;
}

void handleWebClientRequests() {
    server.handleClient();
}

// Root page handler
void handleRoot() {
    String html = "<!DOCTYPE html>";
    html += "<html lang='en'>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<meta http-equiv='refresh' content='5'>"; // Auto-refresh every 5 seconds
    html += "<title>ESP32 Garden Monitor</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; text-align: center; }";
    html += "h1 { color: #008000; }";
    html += ".info { background-color: #f0f0f0; border-radius: 5px; padding: 15px; margin: 20px 0; }";
    html += ".button { background-color: #008000; border: none; color: white; padding: 10px 20px; ";
    html += "text-decoration: none; font-size: 16px; margin: 10px 2px; cursor: pointer; border-radius: 5px; }";
    html += ".sensor { margin: 10px 0; padding: 10px; background-color: #e8f5e9; border-radius: 5px; }";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<h1>ESP32 Garden Monitor</h1>";
    
    html += "<div class='info'>";
    html += "<p><strong>Device Status:</strong> Connected</p>";
    html += "<p><strong>Network:</strong> " + WiFi.SSID() + "</p>";
    html += "<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>";
    html += "<p><strong>Signal Strength:</strong> " + String(WiFi.RSSI()) + " dBm</p>";
    html += "<p><strong>MQTT Status:</strong> " + String(isMQTTConnected() ? "Connected" : "Disconnected") + "</p>";
    html += "</div>";
    
    html += "<h2>Sensor Readings</h2>";
    
    html += "<div class='sensor'>";
    html += "<h3>Environmental Conditions</h3>";
    html += "<p><strong>Temperature:</strong> " + (latestData.dhtValid ? String(latestData.temperature) + " °C" : "Reading error") + "</p>";
    html += "<p><strong>Humidity:</strong> " + (latestData.dhtValid ? String(latestData.humidity) + " %" : "Reading error") + "</p>";
    html += "<p><strong>Light Intensity:</strong> " + (latestData.lightValid ? String(latestData.lightLevel) + " lx" : "Reading error") + "</p>";
    html += "</div>";
    
    html += "<div class='sensor'>";
    html += "<h3>Soil Conditions</h3>";
    html += "<p><strong>Soil Moisture:</strong> " + String(latestData.soilMoisture) + " (Raw ADC value)</p>";
    html += "<p><strong>Water Level:</strong> " + String(latestData.waterLevel) + " (Raw ADC value)</p>";
    html += "</div>";
    
    html += "<div>";
    html += "<a href='/mqtt-config' class='button'>MQTT Settings</a> ";
    html += "<a href='/reset' class='button'>Reset WiFi</a>";
    html += "</div>";
    
    html += "</body>";
    html += "</html>";
    
    server.send(200, "text/html", html);
}
void handleResetWiFi() {
    // Xóa thông tin WiFi đã lưu trong flash
    WiFi.disconnect(true, true); // true, true: xóa thông tin đã lưu và các cài đặt liên quan
    
    String html = "<!DOCTYPE html>";
    html += "<html lang='en'>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>WiFi Reset</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; text-align: center; }";
    html += "h1 { color: #d9534f; }";
    html += ".info { background-color: #f5f5f5; border-radius: 5px; padding: 15px; margin: 20px auto; max-width: 500px; }";
    html += ".spinner { border: 4px solid rgba(0, 0, 0, 0.1); width: 36px; height: 36px; border-radius: 50%; ";
    html += "border-left-color: #09f; animation: spin 1s linear infinite; margin: 20px auto; }";
    html += "@keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<h1>WiFi đã được reset thành công</h1>";
    html += "<div class='info'>";
    html += "<p>Thiết bị sẽ khởi động lại sau 3 giây...</p>";
    html += "<p>Sau khi khởi động lại, thiết bị sẽ vào chế độ cấu hình WiFi mới.</p>";
    html += "<p>Vui lòng kết nối đến mạng WiFi <strong>ESP32_Setup</strong> để cấu hình lại.</p>";
    html += "<div class='spinner'></div>";
    html += "</div>";
    html += "<script>";
    html += "setTimeout(function() {";
    html += "  document.body.innerHTML += '<p>Đang khởi động lại...</p>';";
    html += "}, 2800);";
    html += "</script>";
    html += "</body>";
    html += "</html>";
    
    server.send(200, "text/html", html);
    
    // Đợi để hiển thị thông báo trước khi khởi động lại
    delay(3000);
    
    // Lưu trạng thái reset vào EEPROM hoặc biến toàn cục để biết
    // sau khi khởi động lại cần chuyển sang chế độ AP để cấu hình
    
    ESP.restart(); // Khởi động lại thiết bị để cấu hình WiFi mới
}

// MQTT configuration page
void handleMQTTConfig() {
    String html = "<!DOCTYPE html>";
    html += "<html lang='en'>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>MQTT Configuration</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; text-align: center; }";
    html += "h1 { color: #008000; }";
    html += ".info { background-color: #f0f0f0; border-radius: 5px; padding: 15px; margin: 20px 0; text-align: left; }";
    html += ".button { background-color: #008000; border: none; color: white; padding: 10px 20px; ";
    html += "text-decoration: none; font-size: 16px; margin: 10px 2px; cursor: pointer; border-radius: 5px; }";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<h1>MQTT Configuration</h1>";
    
    html += "<div class='info'>";
    html += "<p><strong>MQTT Server:</strong> " + String(MQTT_SERVER) + "</p>";
    html += "<p><strong>MQTT Port:</strong> " + String(MQTT_PORT) + "</p>";
    html += "<p><strong>MQTT Username:</strong> " + String(MQTT_USER) + "</p>";
    html += "<p><strong>MQTT Status:</strong> " + String(isMQTTConnected() ? "Connected" : "Disconnected") + "</p>";
    html += "<p><strong>Topic Structure:</strong></p>";
    html += "<ul>";
    html += "<li>" + String(TOPIC_TEMPERATURE) + "</li>";
    html += "<li>" + String(TOPIC_HUMIDITY) + "</li>";
    html += "<li>" + String(TOPIC_LIGHT) + "</li>";
    html += "<li>" + String(TOPIC_SOIL) + "</li>";
    html += "<li>" + String(TOPIC_WATER) + "</li>";
    html += "</ul>";
    html += "</div>";
    
    html += "<a href='/' class='button'>Back to Dashboard</a>";
    html += "</body>";
    html += "</html>";
    
    server.send(200, "text/html", html);
}

// Handle 404 errors
void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    
    server.send(404, "text/plain", message);
}