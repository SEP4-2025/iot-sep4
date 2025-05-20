#include "display.h"
#include "mqtt/mqtt.h"
#include "periodic_task.h"
#include "servo.h"
#include "uart.h"
#include "wifi.h"
#include "./sensor_readings.h"
#include "setup/system_setup.h"
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>
#include "MQTTPacket.h"
#include "water_pump.h"
#include "hc_sr04.h"
#include "console/console_operations.h"

static unsigned char callback_buff[256];

void my_event_cb()
{
  // Check if we have valid data
  if (callback_buff[0] == 0)
  {
    uart_send_string_blocking(USART_0, "Empty buffer received\n");
    return;
  }

  // Extract MQTT packet type from first byte
  unsigned char packet_type = (callback_buff[0] >> 4) & 0x0F;

  // Print packet type
  char msg_buf[250];
  sprintf(msg_buf, "MQTT packet received - Type: %d\n", packet_type);
  uart_send_string_blocking(USART_0, msg_buf);

  switch (packet_type)
  {
  case 2:
    uart_send_string_blocking(USART_0, "RECEIVED CONNACK\n");
    break;

  case 3:
  {
    unsigned char dup = 0;
    int qos = 0;
    unsigned char retained = 0;
    unsigned short packetid = 0;
    MQTTString topicName = {0};
    unsigned char *payload = NULL;
    int payloadlen = 0;

    int result = MQTTDeserialize_publish(&dup, &qos, &retained, &packetid,
                                         &topicName, &payload, &payloadlen,
                                         callback_buff, 256);

    if (result == 1)
    {
      // create topic string
      char topic[64] = {0};
      if (topicName.lenstring.len < sizeof(topic))
      {
        memcpy(topic, topicName.lenstring.data, topicName.lenstring.len);
        topic[topicName.lenstring.len] = '\0';
      }

      // Create payload string
      char payload_str[128] = {0};
      if (payloadlen < sizeof(payload_str))
      {
        memcpy(payload_str, payload, payloadlen);
        payload_str[payloadlen] = '\0';
      }

      sprintf(msg_buf, "PUBLISH: Topic='%s', Payload='%s', QoS=%d\n",
              topic, payload_str, qos);
      uart_send_string_blocking(USART_0, msg_buf);

      // Check for pump topic
      if (strcmp(topic, "pump/command") == 0)
      {
        //convert message milliseconds string into int
        char *endptr;
        int duration_ms = (int)strtol(payload, &endptr, 10);

        if (*endptr != '\0') {
          uart_send_string_blocking(USART_0, "Invalid number!\n");
        }
        else {
          char debug_msg[50];
          sprintf(debug_msg, "Duration received: %d ms. Starting the pump...", duration_ms);
          uart_send_string_blocking(USART_0, debug_msg);

          pump_run(duration_ms);
        }
      } 
      else if (strcmp(topic, "pump/command_stop") == 0)
      {
        uart_send_string_blocking(USART_0, "Command received: Turn pump OFF\n");
        pump_stop();
      }
      else if (strcmp(topic, "pump/command_start") == 0)
      {
        uart_send_string_blocking(USART_0, "Command received: Turn pump ON indefinitely\n");
        pump_start();
      }
      
    }
    else
    {
      uart_send_string_blocking(USART_0, "Failed to parse PUBLISH packet\n");
    }
    break;
  }

  case 9: // MQTT SUBACK
    uart_send_string_blocking(USART_0, "RECEIVED SUBACK\n");
    break;

  default:
    sprintf(msg_buf, "Unhandled packet type: %d\n", packet_type);
    uart_send_string_blocking(USART_0, msg_buf);
  }
}

void loop()
{
  send_temperature_humidity_reading();
    // _delay_ms(500);
  // send_soil_moisture_reading();
    // _delay_ms(500);
  // send_light_reading();
}

void loop2() {
  send_soil_moisture_reading();
}

void loop3() {
  send_light_reading();
}

int main()
{
  // Initialize all sensors
  initialize_system();

  // Connect to network and MQTT broker
  if (setup_network_connection("Marius iPhone", "password123", "34.27.128.90", 1883,
                               my_event_cb, callback_buff) != WIFI_OK)
  {
    return -1;
  }

  _delay_ms(2000);

  // subscribe to pump
  WIFI_ERROR_MESSAGE_t subscribe_message = mqtt_subscribe_to_topic("pump/command", 1);
  if (subscribe_message != WIFI_OK)
  {
    uart_send_string_blocking(USART_0, "Unable to send subscribe packet!\n");
  }
  else
  {
    uart_send_string_blocking(USART_0, "Sent subscribe packet!\n");
  }

  _delay_ms(2000);
  // subscribe to pump stop
  WIFI_ERROR_MESSAGE_t subscribe_stop_message = mqtt_subscribe_to_topic("pump/command_stop", 2);
  if (subscribe_stop_message != WIFI_OK)
  {
    uart_send_string_blocking(USART_0, "Unable to send subscribe packet!\n");
  }
  else
  {
    uart_send_string_blocking(USART_0, "Sent subscribe packet!\n");
  }

  _delay_ms(2000);
  // subscribe to pump start
  WIFI_ERROR_MESSAGE_t subscribe_start_message = mqtt_subscribe_to_topic("pump/command_start", 3);
  if (subscribe_start_message != WIFI_OK)
  {
    uart_send_string_blocking(USART_0, "Unable to send subscribe packet!\n");
  }
  else
  {
    uart_send_string_blocking(USART_0, "Sent subscribe packet!\n");
  }

  periodic_task_init_a(loop, 30000);
  periodic_task_init_b(loop2, 40000);
  periodic_task_init_c(loop3, 50000);

  while (1)
  {
  }

  return 0;
}
// int main() {
//   uart_init(USART_0, 9600, console_rx);
//   uart_send_string_blocking(USART_0, "Hello from main!\n");

//   // pump_init();
//   // pump_run(5000);

//   hc_sr04_init();
//   uint16_t test = hc_sr04_takeMeasurement();

//   char msg_buf[250];
//   sprintf(msg_buf, "%d\n", test);
//   uart_send_string_blocking(USART_0, msg_buf);
    
//   while(1) {
//     test = hc_sr04_takeMeasurement();
//     sprintf(msg_buf, "%d\n", test);
//     uart_send_string_blocking(USART_0, msg_buf);
      
//     _delay_ms(2000);
//   }
    
//   return 0;
// }
