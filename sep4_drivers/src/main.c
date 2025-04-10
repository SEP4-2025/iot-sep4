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

int main()
{
    wifi_init();
    light_init();
    sei();
    uart_init(USART_0, 9600, console_rx);
    uart_send_string_blocking(USART_0, "Hello\n");
    wifi_command_create_TCP_connection("10.121.138.177", 1883, NULL, NULL);
    WIFI_ERROR_MESSAGE_t wifi_res = wifi_command_join_AP("Dimitar's Pixel 7 Pro", "1234qwert");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    int rc = 0;
    char buf[200];
    int buflen = sizeof(buf);
    MQTTString topicString = MQTTString_initializer;
    char *payload = "I'm alive!";
    int payloadlen = strlen(payload);
    int len = 0;

    data.clientID.cstring = "mbed test client - Ian Craggs";
    data.keepAliveInterval = 20;
    data.cleansession = 1;
    data.MQTTVersion = 3;

    len = MQTTSerialize_connect(buf, buflen, &data);

    topicString.cstring = "mbed NXP LPC1768";
    len += MQTTSerialize_publish(buf + len, buflen - len, 0, 0, 0, 0, topicString, payload, payloadlen);
    len += MQTTSerialize_disconnect(buf + len, buflen - len);

    wifi_command_TCP_transmit(buf, len);

    char wifi_res_msg[128];
    sprintf(wifi_res_msg, "Error: %d", wifi_res);
    uart_send_string_blocking(USART_0, wifi_res_msg);
    uint16_t light = light_read();
    char light_str[128] = "";
    sprintf(light_str, "Light data: %d", light);
    while (1)
    {
    }
    return 0;
}