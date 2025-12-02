#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "config.h"
#include "WiFiManager.h"
#include "ZenohPublisher.h"

#if Z_FEATURE_PUBLICATION == 1

// Global objects
WiFiManager* wifiManager = nullptr;
ZenohPublisher* zenohPublisher = nullptr;

void setup() {
    // Initialize Serial for debug
    Serial.begin(115200);
    while (!Serial) {
        delay(1000);
    }
    
    Serial.println("\n=== ESP32 Zenoh Publisher ===");
    
    // Initialize WiFi
    wifiManager = new WiFiManager(WIFI_SSID, WIFI_PASS);
    if (!wifiManager->connect()) {
        Serial.println("Failed to connect to WiFi!");
        while (1) {
            delay(1000);
        }
    }
       
    // Initialize Zenoh Publisher
    zenohPublisher = new ZenohPublisher(ZENOH_MODE, ZENOH_LOCATOR, ZENOH_KEYEXPR);
    
    if (!zenohPublisher->initSession()) {
        Serial.println("Failed to initialize Zenoh session!");
        while (1) {
            delay(1000);
        }
    }
    
    if (!zenohPublisher->startTasks()) {
        Serial.println("Failed to start Zenoh tasks!");
        while (1) {
            delay(1000);
        }
    }
    
    if (!zenohPublisher->declarePublisher()) {
        Serial.println("Failed to declare publisher!");
        while (1) {
            delay(1000);
        }
    }
    
    // Start publishing task
    zenohPublisher->startPublishTask(ZENOH_VALUE, PUBLISH_INTERVAL_MS);
    
    Serial.println("=== Setup Complete ===\n");
}

void loop() {
    // Monitor WiFi connection
    if (!wifiManager->isConnected()) {
        Serial.println("WiFi connection lost! Reconnecting...");
        wifiManager->connect();
    }
    
    // Main loop just monitors - all work is done in FreeRTOS tasks
    vTaskDelay(pdMS_TO_TICKS(5000));
}

#else
void setup() {
    Serial.begin(115200);
    Serial.println("ERROR: Zenoh pico was compiled without Z_FEATURE_PUBLICATION but this example requires it.");
}
void loop() {
    delay(1000);
}
#endif