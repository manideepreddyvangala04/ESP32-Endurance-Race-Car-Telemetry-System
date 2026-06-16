#ifndef NETWORK_MODULE_H
#define NETWORK_MODULE_H

#include <WiFi.h>
#include <HTTPClient.h>
#include "Config.h"

class NetworkManager {
public:
    static void connect() {
        if (WiFi.status() == WL_CONNECTED) return;
        
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        Serial.print("Connecting to WiFi");
        
        // Give it a max of 10 seconds to connect so we don't hang forever
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            vTaskDelay(pdMS_TO_TICKS(500));
            Serial.print(".");
            attempts++;
        }
        Serial.println(WiFi.status() == WL_CONNECTED ? "\nConnected!" : "\nWiFi Failed.");
    }

    static void uploadLapData(const LapRecord& data) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Network offline. Upload skipped.");
            return; // Defensive: Don't attempt HTTP if WiFi is down
        }

        HTTPClient http;
        // setInsecure allows connection to HTTPS Google Sheets without verifying SSL certificates,
        // which prevents handshake failures as Google's certs update frequently.
        http.setInsecure(); 

        String url = String(SCRIPT_URL) + 
                     "?lap=" + String(data.lapNumber) +
                     "&time=" + String(data.lapTimeSeconds) +
                     "&long=" + String(data.finalLongitudinalG, 3) +
                     "&lat=" + String(data.finalLateralG, 3) +
                     "&vert=" + String(data.finalVerticalG, 3);

        http.begin(url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            Serial.printf("Cloud Sync Success: HTTP %d\n", httpCode);
        } else {
            Serial.printf("Cloud Sync Failed: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
};

#endif
