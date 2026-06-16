#ifndef CONFIG_H
#define CONFIG_H

// Hardware Pins
constexpr int PIN_BUZZER = 25;
constexpr int PIN_LAP_BTN = 0;
constexpr int I2C_SDA = 21;
constexpr int I2C_SCL = 22;

// ADXL345 Sensor Constants
constexpr int ADXL345_ADDRESS = 0x53;
constexpr float EMA_ALPHA = 0.6f; // Smoothing factor for high-frequency vibration
constexpr float G_FORCE_SPIKE_THRESHOLD = 1.5f;
constexpr float JERK_SPIKE_THRESHOLD = 0.5f;

// Network Credentials
const char* const WIFI_SSID = "ESP32_TEST";
const char* const WIFI_PASS = "12345678";
const char* const SCRIPT_URL = "https://script.google.com/macros/s/AKfycbym-0OCtDO-DTeqNdZtWwxad6e14gfmTeIB2eIo6YbG8j5K3P8E-iwPxe_o9v_-N3Pl/exec";

// Data structure to pass lap info between FreeRTOS tasks safely
struct LapRecord {
    int lapNumber;
    int lapTimeSeconds;
    float finalLongitudinalG;
    float finalLateralG;
    float finalVerticalG;
};

#endif
