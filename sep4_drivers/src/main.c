#include "display.h"
#include "light.h"
#include "mqtt.h"
#include "MQTTPacket.h"
#include "dht11.h"
#include "display.h"
#include "light.h"
#include "periodic_task.h"
#include "uart.h"
#include "wifi.h"
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

static uint8_t _buff[100];
static uint8_t _index = 0;
volatile static bool _done = false;
void console_rx(uint8_t _rx) {
  uart_send_blocking(USART_0, _rx);
  if (('\r' != _rx) && ('\n' != _rx)) {
    if (_index < 100 - 1) {
      _buff[_index++] = _rx;
    }
  } else {
    _buff[_index] = '\0';
    _index = 0;
    _done = true;
    uart_send_blocking(USART_0, '\n');
    //        uart_send_string_blocking(USART_0, (char*)_buff);
  }
}

static void my_event_cb(const void *evt, void *data) {
  /* do stuff and things with the event */
  uart_send_string_blocking(USART_0, "Hello from callback!\n");
}

// Function to create and serialize the MQTT connect packet
void loop() {
  unsigned char transmit_buf[200];
  size_t transmit_buflen = sizeof(transmit_buf);

  char *topic = "light:reading";
  uint16_t light_int = light_read();
  unsigned char payload[20] = "";
  sprintf((char *)payload, "Light data:%d\n", light_int);
  size_t transmit_len = create_mqtt_transmit_packet(
      topic, payload, (unsigned char *)transmit_buf, transmit_buflen);

  if (transmit_len > 0) {
    printf("MQTT Transmit packet created. Length: %d\n", transmit_len);
  }
  wifi_command_TCP_transmit(transmit_buf, transmit_len);
}

int main() {
  // Init wifi and light
  wifi_init();
  light_init();

  // Writing in the console
  uart_init(USART_0, 9600, console_rx);
  uart_send_string_blocking(USART_0, "Hello from main!\n");

  // Connect to wifi network
  WIFI_ERROR_MESSAGE_t wifi_res =
      wifi_command_join_AP("Dimitar_Nizamov", "dn22042002");

  // Connect to TCP server
  // Write callback function to type in the messag ein the uart
  char *_buff = malloc(100);
  wifi_command_create_TCP_connection("192.168.0.116", 1883, my_event_cb,
                                     _buff);

  // Log the result of the wifi connection
  char wifi_res_msg[128];
  sprintf(wifi_res_msg, "Error: %d \n", wifi_res);
  uart_send_string_blocking(USART_0, wifi_res_msg);

  if (wifi_res != WIFI_OK) {
    uart_send_string_blocking(USART_0, "Error connecting to wifi!\n");
    return -1;
  } else {
    uart_send_string_blocking(USART_0, "Connected to wifi!\n");
  }
  unsigned char connect_buf[200];
  int connect_buflen = sizeof(connect_buf);
  int connect_len = create_mqtt_connect_packet(connect_buf, connect_buflen);
  if (connect_len > 0) {
    printf("MQTT Connect packet created. Length: %d\n", connect_len);
  }
  wifi_command_TCP_transmit(connect_buf, connect_len);
  periodic_task_init_a(loop, 2000);
  while (1) {
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
