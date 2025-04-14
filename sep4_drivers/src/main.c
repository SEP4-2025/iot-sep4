#include "wifi.h"
#include "uart.h"
#include "light.h"
#include "display.h"
#include "MQTTPacket.h"
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include "MQTTPacket.h"
#include "periodic_task.h"

static uint8_t _buff[100];
static uint8_t _index = 0;
volatile static bool _done = false;
void console_rx(uint8_t _rx)
{
    uart_send_blocking(USART_0, _rx);
    if (('\r' != _rx) && ('\n' != _rx))
    {
        if (_index < 100 - 1)
        {
            _buff[_index++] = _rx;
        }
    }
    else
    {
        _buff[_index] = '\0';
        _index = 0;
        _done = true;
        uart_send_blocking(USART_0, '\n');
        //        uart_send_string_blocking(USART_0, (char*)_buff);
    }
}

static void my_event_cb(const struct event *evt, void *data)
{
    /* do stuff and things with the event */
    uart_send_string_blocking(USART_0, "Hello from callback!\n");
}

void loop()
{
    char transmit_buf[200];
    int transmit_buflen = sizeof(transmit_buf);

    char topic[] = "light:reading";
    uint16_t light_int = light_read();
    char payload[20] = "";
    sprintf(payload, "Light data:%d\n", light_int);
    int transmit_len = create_mqtt_transmit_packet(
        topic, payload, transmit_buf, transmit_buflen);

    if (transmit_len > 0)
    {
        printf("MQTT Transmit packet created. Length: %d\n", transmit_len);
    }
    wifi_command_TCP_transmit(transmit_buf, transmit_len);
}

int main()
{
    // Init wifi and light
    wifi_init();
    light_init();

    // Writing in the console
    uart_init(USART_0, 9600, console_rx);
    uart_send_string_blocking(USART_0, "Hello from main!\n");

    // Connect to wifi network
    WIFI_ERROR_MESSAGE_t wifi_res = wifi_command_join_AP("Dimitar_Nizamov", "dn22042002");

    // Connect to TCP server
    // Write callback function to type in the messag ein the uart
    char *_buff = (uint8_t *)malloc(100);
    wifi_command_create_TCP_connection("192.168.0.116", 1883, my_event_cb, _buff);
    // uart_send_string_blocking(USART_0, _buff);

    // Log the result of the wifi connection
    char wifi_res_msg[128];
    sprintf(wifi_res_msg, "Error: %d \n", wifi_res);
    uart_send_string_blocking(USART_0, wifi_res_msg);

    if (wifi_res != WIFI_OK)
    {
        uart_send_string_blocking(USART_0, "Error connecting to wifi!\n");
        return -1;
    }
    else
    {
        uart_send_string_blocking(USART_0, "Connected to wifi!\n");
    }
    char connect_buf[200];
    int connect_buflen = sizeof(connect_buf);
    int connect_len = create_mqtt_connect_packet(connect_buf, connect_buflen);
    if (connect_len > 0)
    {
        printf("MQTT Connect packet created. Length: %d\n", connect_len);
    }
    wifi_command_TCP_transmit(connect_buf, connect_len);
    periodic_task_init_a(loop, 2000);
    while (1)
    {
    }
    // char disconnect_buf[200];
    // int disconnect_buflen = sizeof(disconnect_buf);
    // int disconnect_len =
    //     create_mqtt_disconnect_packet(disconnect_buf, disconnect_buflen);
    // if (disconnect_len > 0)
    // {
    //     printf("MQTT Disconnect packet created. Length: %d\n",
    //            disconnect_len);
    // }
    return 0;
}

// Function to create and serialize the MQTT connect packet
int create_mqtt_connect_packet(char *buf, int buflen)
{
    MQTTPacket_connectData data =
        MQTTPacket_connectData_initializer;
    int len = 0;

    data.clientID.cstring = "atmega2560";
    data.keepAliveInterval = 20;
    data.cleansession = 1;
    data.MQTTVersion = 3;

    len = MQTTSerialize_connect(buf, buflen, &data);
    return len;
}

// Function to create and serialize the MQTT publish packet
int create_mqtt_transmit_packet(
    char *topic, char *payload, char *buf, int buflen)
{
    MQTTString topicString = MQTTString_initializer;
    int payloadlen = strlen(payload);
    int len = 0;

    topicString.cstring = topic;
    len = MQTTSerialize_publish(
        buf, buflen, 0, 0, 0, 0, topicString, payload, payloadlen);
    return len;
}

// Function to create and serialize the MQTT disconnect packet
int create_mqtt_disconnect_packet(char *buf, int buflen)
{
    int len = 0;
    len = MQTTSerialize_disconnect(buf, buflen);
    return len;
}
