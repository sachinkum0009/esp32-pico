#ifndef ZENOH_PUBLISHER_H
#define ZENOH_PUBLISHER_H

#include <zenoh-pico.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class ZenohPublisher {
public:
    ZenohPublisher(const char* mode, const char* locator, const char* keyexpr);
    ~ZenohPublisher();
    
    bool initSession();
    bool declarePublisher();
    bool startTasks();
    bool publish(const char* data);
    
    void startPublishTask(const char* value, uint32_t intervalMs);
    void stopPublishTask();
    
    bool isSessionOpen() const { return _sessionOpen; }
    bool isPublisherDeclared() const { return _publisherDeclared; }

private:
    const char* _mode;
    const char* _locator;
    const char* _keyexpr;
    
    z_owned_session_t _session;
    z_owned_publisher_t _publisher;
    
    bool _sessionOpen;
    bool _publisherDeclared;
    
    TaskHandle_t _publishTaskHandle;
    bool _taskRunning;
    
    static void publishTaskImpl(void* parameter);
    
    struct PublishTaskParams {
        ZenohPublisher* instance;
        const char* value;
        uint32_t intervalMs;
    };
};

#endif // ZENOH_PUBLISHER_H
