/*
  Rui Santos
  Complete project details
   - Arduino IDE: https://RandomNerdTutorials.com/esp32-ota-over-the-air-arduino/
   - VS Code: https://RandomNerdTutorials.com/esp32-ota-over-the-air-vs-code/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
//SAÍDAS
#define bomba   4 //Saída - Bomba de água V5
#define resistencia 2 //Saída - Resistência V8
#define secante 13 //Saída - Solenóide líquido secante V17
#define motorfluxoagua 18 //Saída - Motor do Fluxo de água V22
#define enchimento 19 //Saída - Solenóide de enchimento. V27
//ENTRADAS
#define sensorporta 21 //Entrada - Sensor da porta V1
#define sensornvagua 22 //Entrada - Sensor de Nível água V18
#define SENSORTEMP 23 //Entrada - Sensor de Temperatura
#define sensortempmax 32 //Entrada - Sensor de Temperatura Emergência. V23

// Import required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WebSerial.h>

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(SENSORTEMP);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Replace with your network credentials
const char* ssid = "VIA";
const char* password = "123456789";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Set number of outputs inputs
#define NUM_OUTPUTS  5
#define NUM_INPUTS  3

// Assign each GPIO to an output
int outputGPIOs[NUM_OUTPUTS] = {bomba, resistencia, secante, motorfluxoagua, enchimento};
int inputGPIOs[NUM_INPUTS] = {sensorporta, sensornvagua, sensortempmax};

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println(WiFi.localIP());
}

String getOutputStates() {
  JSONVar myArray;
  for (int i = 0; i < NUM_OUTPUTS; i++) {
    myArray["gpios"][i]["output"] = String(outputGPIOs[i]);
    myArray["gpios"][i]["state"] = String(digitalRead(outputGPIOs[i]));
  }
  String jsonString = JSON.stringify(myArray);
  Serial.println(jsonString);
  return jsonString;
}

void notifyClients(String state) {
  ws.textAll(state);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "states") == 0) {
      notifyClients(getOutputStates());
    }
    else {
      int gpio = atoi((char*)data);
      Serial.println(gpio);
      digitalWrite(gpio, !digitalRead(gpio));
      notifyClients(getOutputStates());
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
void recvMsg(uint8_t *data, size_t len) {
  WebSerial.println("Received Data...");
  String d = "";
  for (int i = 0; i < len; i++) {
    d += char(data[i]);
  }
  WebSerial.println(d);
}
void setup() {
  delay(1000);
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Set GPIOs as outputs
  for (int i = 0; i < NUM_OUTPUTS; i++) {
    pinMode(outputGPIOs[i], OUTPUT);
  }
  initSPIFFS();
  initWiFi();
  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html", false);
  });

  server.serveStatic("/", SPIFFS, "/");

  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);
  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  // Start server
  server.begin();
  sensors.begin();
  WebSerial.println("Hello World");
}

void loop() {
  AsyncElegantOTA.loop();
  ws.cleanupClients();
}
