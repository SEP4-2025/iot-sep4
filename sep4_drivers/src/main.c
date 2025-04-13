#include "wifi.h"
#include "uart.h"
#include "light.h"
#include "display.h"
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

void send_light_data_to_tcp() 
{
    // Read the light
    uint16_t light = light_read();
    char light_str[128] = "";

    // Send the light data to the tcp server 
    sprintf(light_str, "Light data: %d", light);
    wifi_command_TCP_transmit((uint8_t *)light_str, strlen(light_str));
    // uart_send_string_blocking(USART_0, "I am in a while loop!\n");
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
    WIFI_ERROR_MESSAGE_t wifi_res = wifi_command_join_AP("Plamen’s iPhone", "plamenisgreat");

    // Connect to TCP server
    // Write callback function to type in the messag ein the uart
    char* _buff = (uint8_t *)malloc(100);
    wifi_command_create_TCP_connection("172.20.10.9", 1337, my_event_cb, _buff);
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
    

    while (1) 
    {
        send_light_data_to_tcp();
        _delay_ms(1000);
    }
        
  
    return 0;
}
