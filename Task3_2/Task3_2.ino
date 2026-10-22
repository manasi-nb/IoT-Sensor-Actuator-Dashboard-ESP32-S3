#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include "lab_page.h"

#define TRIG_PIN 4
#define ECHO_PIN 5
#define LED_PIN 18
#define NUM_LEDS 8

//Wi-Fi AP
const char* apSSID = "ESP32-Lab";
const char* apPassword = "a1234567";

WebServer server(80); //web server

Adafruit_NeoPixel strip(
  NUM_LEDS,
  LED_PIN,
  NEO_GRB + NEO_KHZ800
);

//Store the state of each LED
struct LedState {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t brightness;
};

LedState leds[NUM_LEDS] = {};

// Ultrasonic measurement
float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  //Wait up to 30 ms for an echo
  unsigned long duration =
      pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1.0; // No valid echo
  }

  //speed of sound approx = 0.0343 cm/us
  return duration * 0.0343 / 2.0;
}

// JSON helper
void sendError(int code, const char* message) {
  JsonDocument doc;
  doc["error"] = message;

  String response;
  serializeJson(doc, response);
  server.send(code, "application/json", response);
}

void addLedState(JsonObject obj, int id) {
  obj["id"] = id;
  obj["r"] = leds[id].r;
  obj["g"] = leds[id].g;
  obj["b"] = leds[id].b;
  obj["brightness"] = leds[id].brightness;
}

// Apply one LED to hardware
void applyLed(int id) {
  LedState &s = leds[id];

  // Scale RGB using brightness (0 to 255)
  uint8_t r = (uint16_t)s.r * s.brightness / 255;
  uint8_t g = (uint16_t)s.g * s.brightness / 255;
  uint8_t b = (uint16_t)s.b * s.brightness / 255;

  strip.setPixelColor(id, strip.Color(r, g, b));
  strip.show();
}

// GET /
void handleRoot() {
  server.send_P(200, "text/html", LAB_PAGE);
}

// GET /api/distance 
void handleDistance() {
  float distance = readDistance();

  JsonDocument doc;

  if (distance < 0) {
    doc["distance_cm"] = nullptr;
    doc["valid"] = false;
  } else {
    doc["distance_cm"] = distance;
    doc["valid"] = true;
  }

  String response;
  serializeJson(doc, response);

  server.sendHeader("Cache-Control", "no-store"); //prevent caching

  server.send(200, "application/json", response);

  Serial.print("Distance: ");
  Serial.println(distance);
}

// GET /api/leds
void handleGetLeds() {
  JsonDocument doc;

  JsonArray array = doc["leds"].to<JsonArray>();

  for (int i = 0; i < NUM_LEDS; i++) {
    JsonObject obj = array.add<JsonObject>();
    addLedState(obj, i);
  }

  String response;
  serializeJson(doc, response);

  server.send(200, "application/json", response);
}

//PUT /api/leds/{id} 
void handleUpdateLed(int id) {
  if (id < 0 || id >= NUM_LEDS) {
    sendError(404, "Invalid LED ID"); //error handling - 404 for unknown LED ID
    return;
  }

  JsonDocument input;

  DeserializationError error =
      deserializeJson(input, server.arg("plain"));

  if (error || !input.is<JsonObject>()) {
    sendError(400, "Invalid JSON body"); //error handling
    return;
  }

  const char* keys[] = {"r", "g", "b", "brightness"};
  int values[4];

  for (int i = 0; i < 4; i++) {
    JsonVariant value = input[keys[i]];

    if (!value.is<int>()) {
      sendError(400, "RGB and brightness must be integers");
      return;
    }

    values[i] = value.as<int>();

    if (values[i] < 0 || values[i] > 255) {
      sendError(400, "Values must be between 0 and 255"); //error handling - 400 for out of range fields
      return;
    }
  }

  leds[id].r = values[0];
  leds[id].g = values[1];
  leds[id].b = values[2];
  leds[id].brightness = values[3];

  // Update only the selected LED
  applyLed(id);

  JsonDocument output;
  JsonObject obj = output.to<JsonObject>();
  addLedState(obj, id);

  String response;
  serializeJson(output, response);

  server.send(200, "application/json", response);

  Serial.printf("Updated LED %d\n", id + 1);
}

//dynamic LED routes
void handleNotFound() {
  String path = server.uri();

  if (path.startsWith("/api/leds/")) {
    String idText = path.substring(10);

    // Require exactly one numeric LED ID
    if (idText.length() == 1 &&
        idText[0] >= '0' &&
        idText[0] <= '7') {

      if (server.method() == HTTP_PUT) {
        handleUpdateLed(idText.toInt());
      } else {
        sendError(405, "Method not allowed");
      }
      return;
    }

    sendError(404, "Invalid LED ID");
    return;
  }

  sendError(404, "Route not found");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Ultrasonic sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // NeoPixel
  strip.begin();
  strip.clear();
  strip.show();

  // Create ESP32 Wi-Fi access point
  WiFi.mode(WIFI_AP);

  if (!WiFi.softAP(apSSID, apPassword)) {
    Serial.println("Failed to start Wi-Fi AP");
    return;
  }

  // Display IP address
  IPAddress ip = WiFi.softAPIP();

  Serial.println();
  Serial.println("Wi-Fi AP started!");
  Serial.print("SSID: ");
  Serial.println(apSSID);
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.print("Dashboard URL: http://");
  Serial.print(ip);
  Serial.println("/");

  // Register HTTP handlers
  server.on("/", HTTP_GET, handleRoot);

  server.on("/api/distance", HTTP_GET,
            handleDistance);

  server.on("/api/leds", HTTP_GET,
            handleGetLeds);

  server.onNotFound(handleNotFound);

  server.begin();

  Serial.println("HTTP server started!");
}

void loop() {
  server.handleClient();
  delay(2);
}
