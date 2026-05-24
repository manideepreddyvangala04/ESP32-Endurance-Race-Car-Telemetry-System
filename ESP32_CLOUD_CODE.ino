#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "RTClib.h"

RTC_DS3231 rtc;

// WiFi
const char* ssid = "ESP32_TEST";
const char* password = "12345678";

String serverName = "https://script.google.com/macros/s/AKfycbym-0OCtDO-DTeqNdZtWwxad6e14gfmTeIB2eIo6YbG8j5K3P8E-iwPxe_o9v_-N3Pl/exec";

// ADXL345
int ADXL345 = 0x53;

// Pins
#define BUZZER 25
#define LAP_BTN 0

// Variables
float fx=0, fy=0, fz=0;
float px=0, py=0, pz=0;
float alpha = 0.6;

float gforce, jerk;

// Lap
DateTime lapStart;
int lapCount = 0;

// ===== BUZZER =====
void doubleBeep() {
  tone(BUZZER, 2000);
  delay(150);
  noTone(BUZZER);
  delay(150);
  tone(BUZZER, 2000);
  delay(150);
  noTone(BUZZER);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  Wire.begin(21,22);

  pinMode(BUZZER, OUTPUT);
  pinMode(LAP_BTN, INPUT_PULLUP);

  // ADXL345 init
  Wire.beginTransmission(ADXL345);
  Wire.write(0x2D);
  Wire.write(8);
  Wire.endTransmission();

  // RTC
  rtc.begin();
  lapStart = rtc.now();

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println("System Ready");
}

// ===== LOOP =====
void loop() {

  float x,y,z;

  // Read accelerometer
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true);

  x = (Wire.read() | Wire.read()<<8)/256.0;
  y = (Wire.read() | Wire.read()<<8)/256.0;
  z = (Wire.read() | Wire.read()<<8)/256.0;

  // Filter
  fx = alpha*x + (1-alpha)*fx;
  fy = alpha*y + (1-alpha)*fy;
  fz = alpha*z + (1-alpha)*fz;

  // G-force & jerk
  gforce = sqrt(fx*fx + fy*fy + fz*fz);
  jerk = abs(fx-px)+abs(fy-py)+abs(fz-pz);

  px=fx; py=fy; pz=fz;

  // 🚨 G spike → double beep
  if (gforce > 1.5 && jerk > 0.5) {
    doubleBeep();
  }

  // ===== LAP BUTTON (BOOT) =====
  static int lastBtn = HIGH;
  int btn = digitalRead(LAP_BTN);

  if(lastBtn==HIGH && btn==LOW){

    DateTime now = rtc.now();
    TimeSpan lapTime = now - lapStart;

    lapCount++;

    int totalSeconds = lapTime.totalseconds();

    Serial.print("Lap "); Serial.print(lapCount);
    Serial.print(" Time: "); Serial.println(totalSeconds);

    // SEND TO GOOGLE SHEETS
    sendToCloud(lapCount, totalSeconds, fx, fy, fz);

    lapStart = now;
    delay(500);
  }

  lastBtn = btn;

  // Debug
  Serial.print("Long: "); Serial.print(fx);
  Serial.print(" Lat: "); Serial.print(fy);
  Serial.print(" Vert: "); Serial.println(fz);

  delay(100);
}

// ===== CLOUD FUNCTION =====
void sendToCloud(int lap, int timeSec, float longi, float lat, float vert) {

  String url = serverName;

  url += "?lap=" + String(lap);
  url += "&time=" + String(timeSec);
  url += "&long=" + String(longi,3);
  url += "&lat=" + String(lat,3);
  url += "&vert=" + String(vert,3);

  Serial.println("Sending:");
  Serial.println(url);

  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();

  Serial.print("HTTP: ");
  Serial.println(httpCode);

  http.end();
}
