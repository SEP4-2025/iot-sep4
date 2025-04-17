#include <stddef.h>
size_t create_mqtt_connect_packet(unsigned char *buf, size_t buflen);
size_t create_mqtt_transmit_packet(char *topic, unsigned char *payload,
                                unsigned char *buf, size_t buflen);
size_t create_mqtt_disconnect_packet(unsigned char *buf, size_t buflen);
