#include "secret.hpp" //use the secret-template! put in your wifi details and change the filename to secret.hpp

#include <WiFiNINA.h>
#include <WebSocketClient.h>
#include <ArduinoJson.h>

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// WebSocket server
const char* websocket_server = "arduino-websockets-workshop.onrender.com";
const int websocket_port = 443;  // SSL port for wss://

// Pin definitions
const int FORCE_PIN = A0;
const int YELLOW_LED_PIN = 4;  // D4 - Yellow LED (dimmable)

// WebSocket client (use WiFiSSLClient for SSL/TLS)
WiFiSSLClient wifiClient;
WebSocketClient wsClient = WebSocketClient(wifiClient, websocket_server, websocket_port);

// LED state
int brightness = 0;           // Brightness for yellow LED (0-255)

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
  pinMode(FORCE_PIN, INPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);

  // Initialize LEDs to off
  analogWrite(YELLOW_LED_PIN, 0);

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


    int forceValue = analogRead(FORCE_PIN); // read the force sensor

    if (forceValue > 40) { //
      Serial.println("Button pressed - sending toggle to server");

      //map the forceValue to match the brightness value
      String val = String(map(forceValue, 0, 1023, 0, 255));

      // Send button press to server
      wsClient.beginMessage(TYPE_TEXT);
      wsClient.print("{\"type\":\"brightness\",\"value\":"+ val +"}"); //broadcast that the button was pressed to the server, {"type":"buttonPress}, server toggles buttonState

      int result = wsClient.endMessage();
      Serial.print("Message send result: ");
      Serial.println(result); //printing 0, i guess this is success?
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

    // Get brightness and flashInterval from initial state
    if (doc["state"].containsKey("brightness")) {
      brightness = doc["state"]["brightness"];
      analogWrite(YELLOW_LED_PIN, brightness);
      Serial.print("Initial brightness: ");
      Serial.println(brightness);
    }
  }
  // Check for brightness message
  else if (typeStr == "brightness") {
    brightness = doc["value"];
    analogWrite(YELLOW_LED_PIN, brightness);
    Serial.print("Brightness updated to: ");
    Serial.println(brightness);
  }
}
