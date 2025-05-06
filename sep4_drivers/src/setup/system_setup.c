#include "system_setup.h"
#include "wifi.h"
#include "light.h"
#include "soil.h"
#include "uart.h"
#include "dht11.h"
#include "mqtt/mqtt.h"
#include "console/console_operations.h"
#include <stdio.h>
#include <string.h>
#include "water_pump.h"

void initialize_system(void)
{
    wifi_init();
    light_init();
    soil_init();
    dht11_init();
    pump_init();
    uart_init(USART_0, 9600, console_rx);
    uart_send_string_blocking(USART_0, "Hello from main!\n");
}

WIFI_ERROR_MESSAGE_t setup_network_connection(const char *ssid, const char *password, const char *broker_ip,
                                              int broker_port, void (*callback)(void), unsigned char *callback_buffer)
{
    // connect to wifi
    WIFI_ERROR_MESSAGE_t wifi_res = wifi_command_join_AP(ssid, password);

    // Log result of WiFi connection
    char wifi_res_msg[128];
    sprintf(wifi_res_msg, "Error: %d \n", wifi_res);
    uart_send_string_blocking(USART_0, wifi_res_msg);

    if (wifi_res != WIFI_OK)
    {
        uart_send_string_blocking(USART_0, "Error connecting to WiFi!\n");
        return -1;
    }
    else
    {
        uart_send_string_blocking(USART_0, "Connected to WiFi!\n");
    }

    // connect to tcp server
    wifi_command_create_TCP_connection(broker_ip, broker_port, callback, callback_buffer);

    // Create and send MQTT connect packet
    unsigned char connect_buf[200];
    int connect_buflen = sizeof(connect_buf);
    int connect_len = create_mqtt_connect_packet(connect_buf, connect_buflen);

    if (connect_len > 0)
    {
        uart_send_string_blocking(USART_0, "MQTT Connect packet created!\n");
    }

    WIFI_ERROR_MESSAGE_t mqtt_res = wifi_command_TCP_transmit(connect_buf, connect_len);

    if (mqtt_res != WIFI_OK)
    {
        uart_send_string_blocking(USART_0, "Error sending MQTT Connect packet!\n");
        return mqtt_res;
    }

    return WIFI_OK;
}