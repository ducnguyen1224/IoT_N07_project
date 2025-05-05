#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <IotWebConf.h>
#include <IotWebConfOptionalGroup.h>

#define RELAY1_PIN 7
#define RELAY2_PIN 6
#define RELAY3_PIN 5
#define RELAY4_PIN 4

#define BUTTON1_PIN 0
#define BUTTON2_PIN 1
#define BUTTON3_PIN 2
#define BUTTON4_PIN 3

#define STRING_LEN 128
#define NUMBER_LEN 32

const char deviceId[] = "Device123";  // Unique Device ID
const char thingName[] = "testThing";
const char wifiInitialApPassword[] = "smrtTHNG8266";
#define CONFIG_VERSION "opt1"
#define STATUS_PIN LED_BUILTIN

// Updated MQTT credentials
const char* mqtt_server = "9f2157c861a94427a60f119c9f7370e2.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  // Using TLS/SSL (port 8883)
const char* mqtt_user = "hivemq.webclient.1742874741899";
const char* mqtt_password = "eV7d41D0NG.g:X>icz#H";

WiFiClientSecure espClient; // Using secure client for TLS/SSL
PubSubClient client(espClient);

DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

bool relayStates[4] = {LOW, LOW, LOW, LOW};
bool buttonStates[4] = {HIGH, HIGH, HIGH, HIGH};
bool lastButtonStates[4] = {HIGH, HIGH, HIGH, HIGH};

void handleRoot();
void configSaved();
void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void publishStatus();

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up...");

  // Skip SSL certificate verification
  espClient.setInsecure();
  
  client.setServer(mqtt_server, mqtt_port);  // Set MQTT server and port
  client.setCallback(callback);  // Set callback for incoming messages

  // Set relay pins as output
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);

  // Set button pins as input with internal pull-up resistors
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);

  // Initially turn off all relays
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(RELAY3_PIN, LOW);
  digitalWrite(RELAY4_PIN, LOW);

  // Set up web configuration
  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.init();

  // Set up the root page
  server.on("/", handleRoot);
  server.on("/config", [] { iotWebConf.handleConfig(); });
  server.onNotFound([]() { iotWebConf.handleNotFound(); });

  // Start WiFi configuration
  iotWebConf.doLoop();
  Serial.println("Ready.");
}

void loop() {
  iotWebConf.doLoop();  // Handle web config loop

  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      reconnect();  // Reconnect MQTT if needed
    }
    client.loop();  // Keep the MQTT connection alive
  }

  // Read button states and toggle corresponding relays
  for (int i = 0; i < 4; i++) {
    int buttonPin = i;
    int relayPin = 7 - i;

    buttonStates[i] = digitalRead(buttonPin);

    if (buttonStates[i] == LOW && lastButtonStates[i] == HIGH) {
      relayStates[i] = !relayStates[i];  // Toggle relay state
      digitalWrite(relayPin, relayStates[i]);

      Serial.print("Relay ");
      Serial.print(i + 1);
      Serial.println(relayStates[i] ? " ON" : " OFF");

      publishStatus();  // Publish relay status via MQTT
      delay(50);  // Debounce delay
    }

    lastButtonStates[i] = buttonStates[i];
  }
}

void publishStatus() {
  String payload = "{";
  payload += "\"d\":{";
  payload += "\"Output1\":" + String(relayStates[0]) + ",";
  payload += "\"Output2\":" + String(relayStates[1]) + ",";
  payload += "\"Output3\":" + String(relayStates[2]) + ",";
  payload += "\"Output4\":" + String(relayStates[3]);
  payload += "}";
  payload += "}";

  // Publish to ThinkIOT topics
  client.publish("/ThinkIOT/status", payload.c_str());  // Publish all status to a common topic
  Serial.println("Published: " + payload);  // Debugging log
}

void handleRoot() {
  if (iotWebConf.handleCaptivePortal()) {
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf MQTT Relay</title></head><div>Status page of ";
  s += iotWebConf.getThingName();
  s += "</div>Go to <a href='config'>configure page</a> to change values.</body></html>\n";

  server.send(200, "text/html", s);  // Send web page content
}

void configSaved() {
  Serial.println("Configuration was updated.");
  client.setServer(mqtt_server, mqtt_port);  // Reconnect to MQTT with updated settings
}

void reconnect() {
  int retryCount = 0;
  while (!client.connected() && retryCount < 5) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);

    // Attempt to connect with username and password
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      
      // Subscribe to the ThinkIOT topics as requested
      client.subscribe("/ThinkIOT/RELAY1");
      client.subscribe("/ThinkIOT/RELAY2");
      client.subscribe("/ThinkIOT/RELAY3");
      client.subscribe("/ThinkIOT/RELAY4");
      
      Serial.println("Subscribed to ThinkIOT topics");
      publishStatus(); // Publish initial status after connection
    } else {
      Serial.print("Failed to connect, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);  // Retry after 5 seconds
      retryCount++;
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(message);

  // Handle control messages from new topics
  if (String(topic) == "/ThinkIOT/RELAY1") {
    relayStates[0] = (message == "ON" || message == "1" || message == "true");
    digitalWrite(RELAY1_PIN, relayStates[0]);
  } else if (String(topic) == "/ThinkIOT/RELAY2") {
    relayStates[1] = (message == "ON" || message == "1" || message == "true");
    digitalWrite(RELAY2_PIN, relayStates[1]);
  } else if (String(topic) == "/ThinkIOT/RELAY3") {
    relayStates[2] = (message == "ON" || message == "1" || message == "true");
    digitalWrite(RELAY3_PIN, relayStates[2]);
  } else if (String(topic) == "/ThinkIOT/RELAY4") {
    relayStates[3] = (message == "ON" || message == "1" || message == "true");
    digitalWrite(RELAY4_PIN, relayStates[3]);
  }
  
  // Publish current status after any change
  publishStatus();
}