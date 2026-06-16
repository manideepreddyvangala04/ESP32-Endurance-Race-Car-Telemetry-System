#ifndef SENSOR_MODULE_H
#define SENSOR_MODULE_H

#include <Wire.h>
#include <math.h>
#include "Config.h"

class TelemetrySensor {
private:
    float smoothedLong = 0, smoothedLat = 0, smoothedVert = 0;
    float prevLong = 0, prevLat = 0, prevVert = 0;
    
public:
    float currentGForce = 0;
    float currentJerk = 0;

    bool begin() {
        Wire.begin(I2C_SDA, I2C_SCL);
        
        // Defensive check: Verify sensor is on the bus before writing
        Wire.beginTransmission(ADXL345_ADDRESS);
        if (Wire.endTransmission() != 0) {
            return false; // Sensor failed to initialize
        }

        Wire.beginTransmission(ADXL345_ADDRESS);
        Wire.write(0x2D); // Power control register
        Wire.write(8);    // Enable measurement mode
        Wire.endTransmission();
        return true;
    }

    void updateReadings() {
        Wire.beginTransmission(ADXL345_ADDRESS);
        Wire.write(0x32); // Start of data registers
        Wire.endTransmission(false);
        Wire.requestFrom(ADXL345_ADDRESS, 6, true);

        if (Wire.available() == 6) {
            // Read raw 16-bit values
            float rawX = (Wire.read() | Wire.read() << 8) / 256.0f;
            float rawY = (Wire.read() | Wire.read() << 8) / 256.0f;
            float rawZ = (Wire.read() | Wire.read() << 8) / 256.0f;

            // Exponential Moving Average (EMA) to filter out engine/road vibrations
            smoothedLong = (EMA_ALPHA * rawX) + ((1.0f - EMA_ALPHA) * smoothedLong);
            smoothedLat  = (EMA_ALPHA * rawY) + ((1.0f - EMA_ALPHA) * smoothedLat);
            smoothedVert = (EMA_ALPHA * rawZ) + ((1.0f - EMA_ALPHA) * smoothedVert);

            currentGForce = sqrt((smoothedLong * smoothedLong) + 
                                 (smoothedLat * smoothedLat) + 
                                 (smoothedVert * smoothedVert));
                                 
            currentJerk = abs(smoothedLong - prevLong) + 
                          abs(smoothedLat - prevLat) + 
                          abs(smoothedVert - prevVert);

            prevLong = smoothedLong;
            prevLat = smoothedLat;
            prevVert = smoothedVert;
        }
    }

    // Getters for the current state
    float getLongitudinal() const { return smoothedLong; }
    float getLateral() const { return smoothedLat; }
    float getVertical() const { return smoothedVert; }
};

#endif
