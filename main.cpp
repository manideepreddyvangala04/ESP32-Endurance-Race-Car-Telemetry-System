#include <Arduino.h>
#include "RTClib.h"
#include "Config.h"
#include "SensorModule.h"
#include "NetworkModule.h"

RTC_DS3231 rtc;
TelemetrySensor accelSensor;

// FreeRTOS OS constructs for hardware awareness
QueueHandle_t cloudUploadQueue;
SemaphoreHandle_t beepSemaphore;

// State Variables
DateTime lapStart;
int lapCount = 0;

// =======================================================
// TASK 1: Alert Buzzer (Runs independently, no blocking)
// =======================================================
void TaskBuzzer(void *pvParameters) {
    pinMode(PIN_BUZZER, OUTPUT);
    for (;;) {
        // Wait here indefinitely until a beep is requested
        if (xSemaphoreTake(beepSemaphore, portMAX_DELAY) == pdTRUE) {
            tone(PIN_BUZZER, 2000);
            vTaskDelay(pdMS_TO_TICKS(150)); // Non-blocking delay
            noTone(PIN_BUZZER);
            vTaskDelay(pdMS_TO_TICKS(150));
            tone(PIN_BUZZER, 2000);
            vTaskDelay(pdMS_TO_TICKS(150));
            noTone(PIN_BUZZER);
        }
    }
}

// =======================================================
// TASK 2: Network Upload (Handles slow HTTP requests)
// =======================================================
void TaskNetwork(void *pvParameters) {
    NetworkManager::connect();
    
    LapRecord incomingData;
    for (;;) {
        // Wait until there is lap data in the queue
        if (xQueueReceive(cloudUploadQueue, &incomingData, portMAX_DELAY) == pdPASS) {
            NetworkManager::connect(); // Ensure we are still connected
            NetworkManager::uploadLapData(incomingData);
        }
    }
}

// =======================================================
// TASK 3: Sensor Polling & Lap Logic (Runs continuously)
// =======================================================
void TaskTelemetryControl(void *pvParameters) {
    pinMode(PIN_LAP_BTN, INPUT_PULLUP);
    bool lastBtnState = HIGH;
    
    for (;;) {
        // 1. Read Sensor
        accelSensor.updateReadings();

        // 2. Check for G-Force Spike -> Signal Buzzer Task
        if (accelSensor.currentGForce > G_FORCE_SPIKE_THRESHOLD && 
            accelSensor.currentJerk > JERK_SPIKE_THRESHOLD) {
            xSemaphoreGive(beepSemaphore);
        }

        // 3. Lap Button Logic with simple debounce
        bool btnState = digitalRead(PIN_LAP_BTN);
        if (lastBtnState == HIGH && btnState == LOW) {
            DateTime now = rtc.now();
            TimeSpan lapTime = now - lapStart;
            lapCount++;

            // Package data into our struct
            LapRecord newLap;
            newLap.lapNumber = lapCount;
            newLap.lapTimeSeconds = lapTime.totalseconds();
            newLap.finalLongitudinalG = accelSensor.getLongitudinal();
            newLap.finalLateralG = accelSensor.getLateral();
            newLap.finalVerticalG = accelSensor.getVertical();

            Serial.printf("Lap %d completed in %d seconds.\n", newLap.lapNumber, newLap.lapTimeSeconds);

            // Send to Network Task via Queue (does not block this sensor loop!)
            xQueueSend(cloudUploadQueue, &newLap, 0);

            lapStart = now;
            vTaskDelay(pdMS_TO_TICKS(500)); // Debounce button
        }
        lastBtnState = btnState;

        // Run this loop every 10ms (100Hz)
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

// =======================================================
// SYSTEM SETUP
// =======================================================
void setup() {
    Serial.begin(115200);

    // Initialize Hardware
    if (!accelSensor.begin()) {
        Serial.println("CRITICAL: ADXL345 not found. Check wiring.");
    }
    
    if (!rtc.begin()) {
        Serial.println("CRITICAL: RTC not found.");
    } else {
        lapStart = rtc.now();
    }

    // Initialize FreeRTOS Queues and Semaphores
    cloudUploadQueue = xQueueCreate(5, sizeof(LapRecord)); // Buffer up to 5 laps
    beepSemaphore = xSemaphoreCreateBinary();

    // Create FreeRTOS Tasks
    // xTaskCreate(Function, Name, Stack Size, Params, Priority, Task Handle)
    xTaskCreate(TaskBuzzer, "BuzzerTask", 2048, NULL, 2, NULL);
    xTaskCreate(TaskNetwork, "NetworkTask", 8192, NULL, 1, NULL); // Lower priority for network
    xTaskCreate(TaskTelemetryControl, "TelemetryTask", 4096, NULL, 3, NULL); // Highest priority for sensors
}

void loop() {
    // Empty. FreeRTOS Tasks handle everything.
    vTaskDelete(NULL); 
}
