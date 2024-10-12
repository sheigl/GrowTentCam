#ifndef __MQTTWRAPPER_H__
#define __MQTTWRAPPER_H__

#include <PubSubClient.h>
#include <WiFiClient.h>
#include "settings.h"

class MqttWrapper
{
private:
    PubSubClient pubSubClient;
    WiFiClient espClient;
    Settings settings;
public:

    MqttWrapper();
    ~MqttWrapper();
    bool Connect();
    void Disconnect();
    void Publish(const char* topic, size_t length, const uint8_t *buffer);
    void Publish(const char* topic, const char* message);
    void Loop();
};

#endif // __MQTTWRAPPER_H__