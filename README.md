# IoT-Sensor-Actuator-Dashboard-ESP32-S3
ESP32-S3-based IoT dashboard for real-time ultrasonic distance sensing and independent RGB NeoPixel control through a RESTful HTTP API.

An ESP32-S3-based IoT system that integrates an HC-SR04 ultrasonic distance sensor and an 8-LED WS2812B NeoPixel bar through a web-based dashboard. The system hosts an HTTP web server on the ESP32-S3, allowing users to request distance measurements and independently control the color and brightness of each LED.

## Features

- **Ultrasonic Distance Measurement:** On-demand distance readings in centimeters using the HC-SR04 sensor.
- **Individual RGB LED Control:** Independently configure the color and brightness of eight NeoPixel LEDs.
- **Web-Based Dashboard:** Responsive HTML interface hosted directly on the ESP32-S3.
- **RESTful HTTP API:** GET and PUT endpoints for sensor readings and actuator control.
- **Wi-Fi Access Point Mode:** Direct connection to the ESP32-S3 without requiring an external router.
- **Input Validation:** JSON-based error handling for malformed requests, invalid values, and unknown LED IDs.
- **State Preservation:** Invalid requests do not modify existing LED configurations.

## Hardware Requirements

- ESP32-S3 development board
- HC-SR04 ultrasonic sensor
- WS2812B NeoPixel RGB LED bar (8 LEDs)
- Breadboard and jumper wires
- Suitable power supply
- Resistors for HC-SR04 ECHO voltage divider

## Hardware Connections

| Component | Pin | ESP32-S3 |
|---|---|---|
| HC-SR04 | TRIG | GPIO 4 |
| HC-SR04 | ECHO | GPIO 5 (via voltage divider) |
| HC-SR04 | VCC | 5V |
| HC-SR04 | GND | GND |
| NeoPixel LED Bar | DIN | GPIO 18 |
| NeoPixel LED Bar | 5V | 5V |
| NeoPixel LED Bar | GND | GND |

**Note:** The HC-SR04 ECHO signal must be reduced to a safe 3.3V logic level before connecting to the ESP32-S3.

## Software Requirements

- Arduino IDE
- ESP32 Arduino board package
- Adafruit NeoPixel library
- ArduinoJson library (v7)
- WiFi and WebServer libraries included with the ESP32 board package

## Setup and Usage

1. Clone or download this repository.
2. Open the Arduino sketch in Arduino IDE.
3. Ensure `lab_page.h` is located in the same folder as the `.ino` file.
4. Install the required libraries.
5. Select the ESP32-S3 board and appropriate COM port.
6. Compile and upload the sketch.
7. Open Serial Monitor at 115200 baud.
8. Connect your computer or smartphone to the ESP32 Wi-Fi network.

**Wi-Fi Configuration**

- SSID: `ESP32-Lab`
- Password: `a1234567`
- Dashboard URL: `http://192.168.4.1/`

The IP address is also printed in the Arduino Serial Monitor.

## HTTP API Endpoints

| Method | Endpoint | Description |
|---|---|---|
| GET | `/` | Serves the IoT dashboard |
| GET | `/api/distance` | Returns an ultrasonic distance measurement |
| GET | `/api/leds` | Retrieves the states of all eight LEDs |
| PUT | `/api/leds/{id}` | Updates the color and brightness of an individual LED |

LED IDs range from 0 to 7.

### Example: Distance Measurement

**Request:** `GET /api/distance`

Successful response:

```json
{
  "distance_cm": 25.4,
  "valid": true
}
```

Invalid measurement:

```json
{
  "distance_cm": null,
  "valid": false
}
```

The sensor uses a finite timeout of 30 milliseconds to avoid indefinite blocking.

### Example: Retrieve LED States

**Request:** `GET /api/leds`

Returns a JSON array containing the state of each LED:

```json
{
  "leds": [
    {
      "id": 0,
      "r": 255,
      "g": 0,
      "b": 0,
      "brightness": 128
    }
  ]
}
```

The example shows one LED state; the actual response contains all eight.

### Example: Update an LED

**Request:** `PUT /api/leds/0`

Request body:

```json
{
  "r": 255,
  "g": 0,
  "b": 0,
  "brightness": 128
}
```

The ESP32 updates LED 0 to red at approximately 50% brightness without modifying the other LEDs.

### Error Handling

| HTTP Status | Meaning |
|---|---|
| 200 | Successful request |
| 400 | Malformed JSON or invalid RGB/brightness values |
| 404 | Unknown LED ID or endpoint |
| 405 | Unsupported HTTP method for an LED endpoint |

Error responses use the format:

```json
{
  "error": "Invalid LED ID"
}
```

## System Operation

The ESP32-S3 runs a synchronous HTTP web server using `server.handleClient()`.

1. The browser sends an HTTP request to the ESP32-S3.
2. The server identifies the requested endpoint.
3. Sensor requests trigger an HC-SR04 measurement.
4. LED update requests undergo input validation before hardware modification.
5. The server returns the result as an HTTP response.

The ultrasonic sensor uses `pulseIn()` with a finite timeout, while the NeoPixel bar is updated using `strip.show()`. Both are synchronous operations.

## Testing

The system is designed to support the following tests:

- Requesting individual ultrasonic measurements.
- Displaying an invalid-reading message when no echo is detected.
- Configuring all eight LEDs with different colors and brightness levels.
- Updating one LED while preserving the remaining seven.
- Verifying successful API responses using an HTTP client.
- Rejecting invalid RGB or brightness values with HTTP 400.
- Rejecting unknown LED IDs with HTTP 404.
- Confirming that rejected requests do not modify stored LED states.

## Project Structure

```text
IoT-Sensor-Actuator-Dashboard-ESP32-S3/
├── Task3_2/
│   ├── Task3_2.ino
│   └── lab_page.h
├── flowchart.png
└── README.md
```

## Technologies Used

- ESP32-S3
- Arduino / C++
- HTML, CSS, JavaScript
- HTTP / REST API
- JSON
- Wi-Fi Access Point
- HC-SR04 Ultrasonic Sensor
- NeoPixel RGB LED strip

Developed as part of an Internet of Things laboratory exercise to demonstrate embedded HTTP web servers, sensor-actuator integration, RESTful communication, JSON data exchange, and synchronous request handling.
