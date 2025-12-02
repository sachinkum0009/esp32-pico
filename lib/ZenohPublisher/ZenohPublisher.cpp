#include "ZenohPublisher.h"
#include <Arduino.h>
#include <string.h>

ZenohPublisher::ZenohPublisher(const char* mode, const char* locator, const char* keyexpr)
    : _mode(mode), _locator(locator), _keyexpr(keyexpr),
      _sessionOpen(false), _publisherDeclared(false),
      _publishTaskHandle(NULL), _taskRunning(false) {
}

ZenohPublisher::~ZenohPublisher() {
    stopPublishTask();
    
    if (_publisherDeclared) {
        z_undeclare_publisher(z_publisher_move(&_publisher));
    }
    
    if (_sessionOpen) {
        zp_stop_read_task(z_session_loan_mut(&_session));
        zp_stop_lease_task(z_session_loan_mut(&_session));
        z_session_drop(z_session_move(&_session));
    }
}

bool ZenohPublisher::initSession() {
    // Initialize Zenoh config
    z_owned_config_t config;
    z_config_default(&config);
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_MODE_KEY, _mode);
    
    if (strcmp(_locator, "") != 0) {
        if (strcmp(_mode, "client") == 0) {
            zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, _locator);
        } else {
            zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_LISTEN_KEY, _locator);
        }
    }
    
    // Open session
    Serial.print("Opening Zenoh Session...");
    if (z_open(&_session, z_config_move(&config), NULL) < 0) {
        Serial.println("FAILED");
        return false;
    }
    Serial.println("OK");
    _sessionOpen = true;
    
    return true;
}

bool ZenohPublisher::startTasks() {
    if (!_sessionOpen) {
        Serial.println("Session not open!");
        return false;
    }
    
    Serial.print("Starting Zenoh read and lease tasks...");
    if (zp_start_read_task(z_session_loan_mut(&_session), NULL) < 0 || 
        zp_start_lease_task(z_session_loan_mut(&_session), NULL) < 0) {
        Serial.println("FAILED");
        return false;
    }
    Serial.println("OK");
    
    return true;
}

bool ZenohPublisher::declarePublisher() {
    if (!_sessionOpen) {
        Serial.println("Session not open!");
        return false;
    }
    
    Serial.print("Declaring publisher for '");
    Serial.print(_keyexpr);
    Serial.print("'...");
    
    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str_unchecked(&ke, _keyexpr);
    
    if (z_declare_publisher(z_session_loan(&_session), &_publisher, 
                           z_view_keyexpr_loan(&ke), NULL) < 0) {
        Serial.println("FAILED");
        return false;
    }
    
    Serial.println("OK");
    _publisherDeclared = true;
    
    return true;
}

bool ZenohPublisher::publish(const char* data) {
    if (!_publisherDeclared) {
        Serial.println("Publisher not declared!");
        return false;
    }
    
    z_owned_bytes_t payload;
    z_bytes_copy_from_str(&payload, data);
    
    if (z_publisher_put(z_publisher_loan(&_publisher), z_bytes_move(&payload), NULL) < 0) {
        Serial.println("Error while publishing data");
        return false;
    }
    
    return true;
}

void ZenohPublisher::publishTaskImpl(void* parameter) {
    PublishTaskParams* params = static_cast<PublishTaskParams*>(parameter);
    ZenohPublisher* instance = params->instance;
    const char* value = params->value;
    uint32_t intervalMs = params->intervalMs;
    
    int idx = 0;
    char buf[256];
    
    while (instance->_taskRunning) {
        sprintf(buf, "[%4d] %s", idx++, value);
        
        Serial.print("Publishing: '");
        Serial.print(buf);
        Serial.println("'");
        
        instance->publish(buf);
        
        vTaskDelay(pdMS_TO_TICKS(intervalMs));
    }
    
    delete params;
    vTaskDelete(NULL);
}

void ZenohPublisher::startPublishTask(const char* value, uint32_t intervalMs) {
    if (_taskRunning) {
        Serial.println("Publish task already running!");
        return;
    }
    
    _taskRunning = true;
    
    PublishTaskParams* params = new PublishTaskParams{this, value, intervalMs};
    
    xTaskCreate(
        publishTaskImpl,
        "ZenohPublishTask",
        4096,
        params,
        1,
        &_publishTaskHandle
    );
    
    Serial.println("Publish task created!");
}

void ZenohPublisher::stopPublishTask() {
    if (_taskRunning) {
        _taskRunning = false;
        if (_publishTaskHandle != NULL) {
            vTaskDelay(pdMS_TO_TICKS(100)); // Give task time to exit
            _publishTaskHandle = NULL;
        }
    }
}
