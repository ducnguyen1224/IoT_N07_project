#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>  // Dùng WiFiClientSecure để hỗ trợ TLS
#include <PubSubClient.h>

// Define pins
const int led1 = 2;
const int led2 = 4;
const int led3 = 5;
const int button1 = 14;
const int button2 = 27;
const int button3 = 26;
const int button4 = 25; 
const int servoPin = 13; 

// LED state variables
bool led1State = false;
bool led2State = false;
bool led3State = false;

// Button previous states for edge detection
bool lastButton1State = HIGH;
bool lastButton2State = HIGH;
bool lastButton3State = HIGH;
bool lastButton4State = HIGH;

// Servo configuration
Servo myServo;
bool servoState = false; // Track servo state (on/off)

// WiFi credentials
const char* ssid = "PTIT_WIFI";
const char* password = "";

// HiveMQ Cloud MQTT Broker
const char* mqtt_server = "9f2157c861a94427a60f119c9f7370e2.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  // Dùng TLS/SSL (port 8883)
const char* mqtt_user = "hivemq.webclient.1742874741899";
const char* mqtt_password = "eV7d41D0NG.g:X>icz#H";

// WiFi client with TLS support
WiFiClientSecure espClient;
PubSubClient client(espClient);

// MQTT topics
const char* topic_led1 = "esp32/led1";
const char* topic_led2 = "esp32/led2";
const char* topic_led3 = "esp32/led3";
const char* topic_servo = "esp32/servo";

void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi.");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  // Control LEDs and Servo via MQTT
  if (String(topic) == topic_led1) {
    led1State = (message == "true");
    digitalWrite(led1, led1State);
    Serial.println(led1State ? "LED 1 ON" : "LED 1 OFF");
  }
  if (String(topic) == topic_led2) {
    led2State = (message == "true");
    digitalWrite(led2, led2State);
    Serial.println(led2State ? "LED 2 ON" : "LED 2 OFF");
  }
  if (String(topic) == topic_led3) {
    led3State = (message == "true");
    digitalWrite(led3, led3State);
    Serial.println(led3State ? "LED 3 ON" : "LED 3 OFF");
  }
  if (String(topic) == topic_servo) {
    servoState = (message == "true");
    myServo.write(servoState ? 180 : 0);
    Serial.println(servoState ? "Servo ON" : "Servo OFF");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Kích hoạt SSL/TLS
    espClient.setInsecure(); // Bỏ qua kiểm tra chứng chỉ

    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT");
      client.publish("esp32/status", "ESP32 is online");
      
      // Subscribe to topics
      client.subscribe(topic_led1);
      client.subscribe(topic_led2);
      client.subscribe(topic_led3);
      client.subscribe(topic_servo);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(". Trying again in 5 seconds.");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);

  // Setup Servo
  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, 500, 2400);
  delay(100);
  myServo.write(0);  // Set initial position

  setup_wifi();

  // Sử dụng HiveMQ Cloud (TLS)
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read button states
  bool button1State = digitalRead(button1);
  bool button2State = digitalRead(button2);
  bool button3State = digitalRead(button3);
  bool button4State = digitalRead(button4);

  // Toggle LED 1 with debounce
  if (button1State == LOW && lastButton1State == HIGH) {
    delay(50); // Debounce
    led1State = !led1State;
    digitalWrite(led1, led1State);
    client.publish(topic_led1, led1State ? "true" : "false");
  }

  // Toggle LED 2 with debounce
  if (button2State == LOW && lastButton2State == HIGH) {
    delay(50); // Debounce
    led2State = !led2State;
    digitalWrite(led2, led2State);
    client.publish(topic_led2, led2State ? "true" : "false");
  }

  // Toggle LED 3 with debounce
  if (button3State == LOW && lastButton3State == HIGH) {
    delay(50); // Debounce
    led3State = !led3State;
    digitalWrite(led3, led3State);
    client.publish(topic_led3, led3State ? "true" : "false");
  }

  // Toggle Servo with debounce
  if (button4State == LOW && lastButton4State == HIGH) {
    delay(50); // Debounce
    servoState = !servoState;
    myServo.write(servoState ? 180 : 0);
    client.publish(topic_servo, servoState ? "true" : "false");
  }

  // Update last button states
  lastButton1State = button1State;
  lastButton2State = button2State;
  lastButton3State = button3State;
  lastButton4State = button4State;
}
