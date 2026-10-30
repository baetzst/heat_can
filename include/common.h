#ifndef COMMON
#define COMMON

#define HOSTNAME "ESP-burner"
#define TOPIC_PREFIX "/heizung/burner/"

#include <PubSubClient.h>
extern PubSubClient mqttClient;

// Publish helpers: on any incomplete write the TCP connection is dropped,
// because a partially sent packet desyncs the MQTT stream on the broker side.
bool mqttPublishRaw(const char* topic, const uint8_t* data, size_t len, bool retain);
bool mqttPublish(const char* topic, const char* msg, bool retain = false);
#endif /* COMMON */
