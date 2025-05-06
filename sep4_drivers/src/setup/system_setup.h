#pragma once

#include "wifi.h"

void initialize_system(void);
WIFI_ERROR_MESSAGE_t setup_network_connection(const char *ssid, const char *password, const char *broker_ip,
                                              int broker_port, void (*callback)(void), unsigned char *callback_buffer);