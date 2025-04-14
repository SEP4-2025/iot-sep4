#include "MQTTPacket.h"
#include <string.h>

size_t create_mqtt_connect_packet(unsigned char *buf, size_t buflen) {
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  size_t len = 0;

  data.clientID.cstring = "atmega2560";
  data.keepAliveInterval = 20;
  data.cleansession = 1;
  data.MQTTVersion = 3;

  len = MQTTSerialize_connect(buf, buflen, &data);
  return len;
}

// Function to create and serialize the MQTT publish packet
size_t create_mqtt_transmit_packet(char *topic, unsigned char *payload,
                                   unsigned char *buf, size_t buflen) {
  MQTTString topicString = MQTTString_initializer;
  size_t payloadlen = strlen((char *)payload);
  size_t len = 0;

  topicString.cstring = topic;
  len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, payload,
                              payloadlen);
  return len;
}

// Function to create and serialize the MQTT disconnect packet
size_t create_mqtt_disconnect_packet(unsigned char *buf, size_t buflen) {
  size_t len = 0;
  len = MQTTSerialize_disconnect(buf, buflen);
  return len;
}
