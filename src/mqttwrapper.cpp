#include "mqttwrapper.h"

MqttWrapper::MqttWrapper() : pubSubClient(), espClient()
{
    pubSubClient.setClient(espClient);
    pubSubClient.setServer(settings.host, 1883);
}

MqttWrapper::~MqttWrapper()
{

}

bool MqttWrapper::Connect()
{
  while (!pubSubClient.connected())
  {
    Serial.println("Connecting to message broker...");
    pubSubClient.connect(settings.hostName, settings.mqUser, settings.mqPassword);

    delay(250);
  }

  return pubSubClient.connected();
}

void MqttWrapper::Disconnect()
{
    pubSubClient.disconnect();
}

void MqttWrapper::Publish(const char *topic, size_t length, const uint8_t *buffer)
{
    String strTopicBase(settings.mqTopicBase);
    String strTopic(topic);
    String strFullTopic(strTopicBase + "/" + strTopic);
    char *topicBuffer = (char*)malloc(strFullTopic.length()+ sizeof(char*));

    strFullTopic.toCharArray(topicBuffer, strFullTopic.length()+ sizeof(char*));

    Serial.print("Sending payload to: ");
    Serial.println(topicBuffer);

    pubSubClient.beginPublish(topicBuffer, length, false);
    pubSubClient.write(buffer, length);
    pubSubClient.endPublish();

    free(topicBuffer);
}

void MqttWrapper::Publish(const char* topic, const char* message)
{
    String strTopicBase(settings.mqTopicBase);
    String strTopic(topic);
    String strFullTopic(strTopicBase + "/" + strTopic);
    char *topicBuffer = (char*)malloc(strFullTopic.length() + sizeof(char*));

    Serial.print("Sending payload to: ");
    Serial.println(topicBuffer);

    strFullTopic.toCharArray(topicBuffer, strFullTopic.length()+ sizeof(char*));
    pubSubClient.publish(topicBuffer, message, String(message).length());

    free(topicBuffer);
}

void MqttWrapper::Loop()
{
    pubSubClient.loop();
}