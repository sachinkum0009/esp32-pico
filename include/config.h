#ifndef CONFIG_H_
#define CONFIG_H_

// WiFi credentials
#define WIFI_SSID "***"
#define WIFI_PASS "***"

// Zenoh configuration
#define ZENOH_MODE "client"
#define ZENOH_LOCATOR "tcp/192.168.0.118:7447"

// Publishing configuration
#define ZENOH_KEYEXPR "demo/example/zenoh-pico-pub"
#define ZENOH_VALUE "[ARDUINO]{ESP32} Publication from Zenoh-Pico!"
#define PUBLISH_INTERVAL_MS 1000

#endif  // CONFIG_H_