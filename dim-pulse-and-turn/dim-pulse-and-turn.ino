#include "secret.hpp" //use the secret-template! put in your wifi details and change the filename to secret.hpp

#include <WiFiNINA.h>
#include <WebSocketClient.h>
#include <ArduinoJson.h>
#include <Servo.h>

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// WebSocket server
const char* websocket_server = "arduino-websockets-workshop.onrender.com";
const int websocket_port = 443;  // SSL port for wss://

// Pin definitions
const int BUTTON_PIN = 2;
const int BLUE_LED_PIN = 3;  // D3 - Blue LED
const int YELLOW_LED_PIN = 4;  // D4 - Yellow LED (dimmable)
const int GREEN_LED_PIN = 5;   // D5 - Green LED (flashing)
const int SERVO_PIN = 6;       // D6 - Servo motor

// WebSocket client (use WiFiSSLClient for SSL/TLS)
WiFiSSLClient wifiClient;
WebSocketClient wsClient = WebSocketClient(wifiClient, websocket_server, websocket_port);

// Servo object
Servo myServo;

// Button debouncing
bool lastButtonState = LOW;
bool buttonPressed = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// LED state
bool ledState = false;
int brightness = 0;           // Brightness for yellow LED (0-255)
int pulseInterval = 0;        // Flash interval for green LED in milliseconds (0 = off)
bool greenLedState = false;   // Current state of green LED
unsigned long lastPulseTime = 0;  // Last time green LED was toggled
int servoPosition = 90;       // Servo position (0-180 degrees)

// Connection tracking
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000;

// Keepalive tracking
unsigned long lastHeartbeat = 0;
const unsigned long heartbeatInterval = 30000; // Send heartbeat every 30 seconds

void setup() {
  Serial.begin(9600);
  /* while (!Serial) { */
  /*   ; // Wait for serial port to connect */
  /* } */

  // Setup pins
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  // Initialize LEDs to off
  digitalWrite(BLUE_LED_PIN, LOW);
  analogWrite(YELLOW_LED_PIN, 0);
  digitalWrite(GREEN_LED_PIN, LOW);

  // Initialize servo
  myServo.attach(SERVO_PIN);
  myServo.write(servoPosition);  // Set to center position (90 degrees)

  // Connect to WiFi
  connectWiFi();

  // Connect to WebSocket server
  connectWebSocket();
}

void loop() {
  if (wsClient.connected()) { // Check if connected
    
    // Check for incoming messages
    int messageSize = wsClient.parseMessage();
    if (messageSize > 0) { //if we have a message, do this!
      Serial.print("Incoming message size: ");
      Serial.println(messageSize);

      String message = ""; //start with an empty string
      while (wsClient.available()) { //while there are messages to read
        message += (char)wsClient.read(); //read the message and concatenate to our message string
      }

      if (message.length() > 0) {  //if there is anything in the message
        handleMessage(message); //call our helper fuction to parse the message
      }
    }

    // Check button state with debouncing
    bool currentButtonState = digitalRead(BUTTON_PIN);

    if (currentButtonState != lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) { //if is an actual button press
      if (currentButtonState == HIGH && !buttonPressed) { //don't fire on held down buttons, check if the button is down and wasn't on the last loop
        buttonPressed = true;
        Serial.println("Button pressed - sending toggle to server");

        // Send button press to server
        wsClient.beginMessage(TYPE_TEXT);
        wsClient.print("{\"type\":\"buttonPress\"}"); //broadcast that the button was pressed to the server, {"type":"buttonPress}, server toggles buttonState
        int result = wsClient.endMessage();
        Serial.print("Message send result: ");
        Serial.println(result); //printing 0, i guess this is success?
      } else if (currentButtonState == LOW) { //was the button unpressed
        buttonPressed = false; //ready for the next press
      }
    }

    lastButtonState = currentButtonState; //for the next loop to check

    // Handle green LED flashing based on flashInterval
    if (pulseInterval > 0) {
      if (millis() - lastPulseTime >= pulseInterval) {
        greenLedState = !greenLedState;
        digitalWrite(GREEN_LED_PIN, greenLedState ? HIGH : LOW);
        lastPulseTime = millis();
      }
    }

    // Send periodic heartbeat to keep connection alive
    if (millis() - lastHeartbeat > heartbeatInterval) {
      Serial.println("Sending heartbeat ping...");
      wsClient.beginMessage(TYPE_TEXT);
      wsClient.print("{\"type\":\"ping\"}"); //sends {"type":"ping"} to the server, which is not broadcast to clients
      wsClient.endMessage();
      lastHeartbeat = millis();
    }
  } else {
    // Try to reconnect
    Serial.print("Connection status: ");
    Serial.println(wsClient.connected() ? "Connected" : "Disconnected");
    if (millis() - lastReconnectAttempt > reconnectInterval) {
      Serial.println("WebSocket disconnected, reconnecting...");
      connectWebSocket();
      lastReconnectAttempt = millis();
    }
  }

  delay(10);
}

//this funcition handles connecting to wifi
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// this function handles connecting to the socket server
void connectWebSocket() {
  Serial.print("Connecting to WebSocket server: ");
  Serial.print(websocket_server);
  Serial.print(":");
  Serial.println(websocket_port);

  wsClient.begin();

  if (wsClient.connected()) {
    Serial.println("WebSocket connected!");
    lastHeartbeat = millis(); // Reset heartbeat timer on new connection
  } else {
    Serial.println("WebSocket connection failed!");
  }
}

//help function to parse the message from ther server
//called whenever a message is received
void handleMessage(String message) {
  Serial.print("Received: ");
  Serial.println(message);

  // Parse JSON using ArduinoJson
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Extract the type field
  const char* type = doc["type"];

  // Check for initialState message
  String typeStr = String(type); //typecast the char* to a string for comparision
  if (typeStr == "initialState") { //all the data
    ledState = doc["state"]["ledOn"]; //update led state to match server state
    digitalWrite(BLUE_LED_PIN, ledState ? HIGH : LOW); //ternery, handle the light value accordingly
    Serial.print("Initial LED state: ");
    Serial.println(ledState ? "ON" : "OFF");

    // Get brightness and flashInterval from initial state
    if (doc["state"].containsKey("brightness")) {
      brightness = doc["state"]["brightness"];
      analogWrite(YELLOW_LED_PIN, brightness);
      Serial.print("Initial brightness: ");
      Serial.println(brightness);
    }

    if (doc["state"].containsKey("pulse")) {
      pulseInterval = doc["state"]["pulse"];
      Serial.print("Initial pulse interval: ");
      Serial.println(pulseInterval);
    }

    if (doc["state"].containsKey("servo")) {
      servoPosition = doc["state"]["servo"];
      servoPosition = constrain(servoPosition, 0, 180);
      myServo.write(servoPosition);
      Serial.print("Initial servo position: ");
      Serial.println(servoPosition);
    }
  }
  // Check for ledState message
  else if (typeStr == "ledState") {
    ledState = doc["value"]; //parse the led state from the returned json.
    digitalWrite(BLUE_LED_PIN, ledState ? HIGH : LOW); //ternery, handle the light value accordingly
    Serial.print("LED toggled to: ");
    Serial.println(ledState ? "ON" : "OFF");
  }
  // Check for brightness message
  else if (typeStr == "brightness") {
    brightness = doc["value"];
    analogWrite(YELLOW_LED_PIN, brightness);
    Serial.print("Brightness updated to: ");
    Serial.println(brightness);
  }
  // Check for flashInterval message
  else if (typeStr == "pulse") {
    pulseInterval = doc["value"];
    Serial.print("pulse interval updated to: ");
    Serial.println(pulseInterval);

    // Reset flash timer when interval changes
    lastPulseTime = millis();
    if (pulseInterval == 0) {
      digitalWrite(GREEN_LED_PIN, LOW);
      greenLedState = false;
    }
  }
  // Check for servo message
  else if (typeStr == "servo") {
    servoPosition = doc["value"];
    // Constrain value to valid servo range (0-180)
    servoPosition = constrain(servoPosition, 0, 180);
    myServo.write(servoPosition);
    Serial.print("Servo position updated to: ");
    Serial.println(servoPosition);
  }
}
