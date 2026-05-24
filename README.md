# RACEPULSE Telemetry System

A low-cost, Wi-Fi enabled endurance race car telemetry system built with an ESP32 and an ADXL345 accelerometer. It tracks real-time driving data, lap times, and G-forces, sending everything directly to a Google Sheet.

## Features
* **Cloud Logging:** Sends lap times and 3-axis motion data directly to Google Sheets via Wi-Fi.
* **G-Force Alarm:** Triggers a loud double-beep if it detects an aggressive G-force spike (over 1.5G and 0.5 jerk).
* **Real-Time Lap Timing:** Uses an onboard RTC module for precise lap tracking.
* **Stress Analytics:** Tracks total G-force and calculates tire stress and kerb impacts based on 3-axis data.
* **Manual Lap Trigger:** Click the onboard button to log a completed lap and push data to the cloud.

## Hardware Requirements
* Microcontroller: ESP32 Development Board
* Sensor: ADXL345 Accelerometer Module
* Timing: DS3231 RTC Module
* Outputs: Active Buzzer
* Inputs: 1x Tactile Push Button

## Wiring Schematic
*Note: The ESP32 uses 3.3V logic. Do not connect the ADXL345 or RTC to a 5V source.*

| Component      |      ESP32 Pin      |                Notes            |
|----------------|---------------------|---------------------------------|
| ADXL345 VCC    | `3.3V`              | Main power                      |
| ADXL345 GND    | `GND`               | Common ground                   |
| ADXL345 SDA    | `GPIO 21`           | I2C Data                        |
| ADXL345 SCL    | `GPIO 22`           | I2C Clock                       |
| RTC VCC        | `3.3V`              | Shared main power               |
| RTC GND        | `GND`               | Shared common ground            |
| RTC SDA        | `GPIO 21`           | Shared I2C Data                 |
| RTC SCL        | `GPIO 22`           | Shared I2C Clock                |
| Buzzer (+)     | `GPIO 25`           | Wire the other leg to GND       |
| Lap Button     | `GPIO 0`            | Wire button diagonally to GND   |


*(Note: Internal pull-up resistors are used for the button; no external resistors are needed.)*


## Software Setup Instructions

### 1. Arduino IDE Configuration
Install the ESP32 board definitions and select these settings under the **Tools** menu:
* **Board:** `ESP32 Dev Module`
* **Wi-Fi Setup:** Update the `ssid` and `password` variables in the code with your network credentials.
* **Cloud Setup:** Update the `serverName` variable in the code with your Google Apps Script URL.

### 2. Required Libraries
Install via the Arduino Library Manager:
* `RTClib` (by Adafruit)
* `WiFi` (Built-in to ESP32 core)
* `HTTPClient` (Built-in to ESP32 core)

## Usage Guide
1. **Initialization:** On power-up, the device will automatically connect to your Wi-Fi network.
2. **Start Lap:** Press the Lap Button once to initialize the timer.
3. **Driving Feedback:** Drive the vehicle. If you hit a massive bump or corner too aggressively (G-force > 1.5, Jerk > 0.5), the buzzer will sound a double-beep alarm.
4. **Log Data:** Every time you cross the finish line, press the Lap Button. The system will calculate your lap time and send the telemetry data directly to your Google Sheet.
