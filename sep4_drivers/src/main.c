#include "wifi.h"
#include "uart.h"
#include "light.h"
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
    char welcome_text[] = "Welcome from SEP4 IoT hardware!\n";
    char prompt_text[] = "Type text to send: ";
    wifi_init();
    light_init();
    sei();
    WIFI_ERROR_MESSAGE_t wifi_res = wifi_command_join_AP("Dimitar's Pixel 7 Pro", "1234qwert");
    wifi_command_create_TCP_connection("10.121.138.177", 1337, NULL, NULL);
    char wifi_res_msg[128];
    sprintf(wifi_res_msg, "Error: %d", wifi_res);
    uart_send_string_blocking(USART_0, wifi_res_msg);
    uint16_t light = light_read();
    char light_str[128] = "";
    sprintf(light_str, "Light data: %d", light);
    wifi_command_TCP_transmit((uint8_t *)light_str, strlen(light_str));

    while (1)
    {
    }
    return 0;
}