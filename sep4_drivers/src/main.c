#include "display.h"
#include "light.h"
#include "mqtt/mqtt.h"
#include "periodic_task.h"
#include "servo.h"
#include "soil.h"
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
#include "MQTTPacket.h"
#include <string.h>
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
  case 2: // MQTT CONNACK
    uart_send_string_blocking(USART_0, "RECEIVED CONNACK\n");
    break;

  case 3: // MQTT PUBLISH
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
      if (strcmp(topic, "pump:command") == 0)
      {
        uart_send_string_blocking(USART_0, "Command received: Turn pump ON\n");

        // Pump code here
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
  send_soil_moisture_reading();
  send_temperature_humidity_reading();
  send_light_reading();
}

int main()
{
  // Initialize all sensors
  initialize_system();

  // Connect to network and MQTT broker
  if (setup_network_connection("Marius iPhone", "password123", "172.20.10.4", 1883,
                               my_event_cb, callback_buff) != WIFI_OK)
  {
    return -1;
  }

  _delay_ms(5000);

  // subscribe to pump
  WIFI_ERROR_MESSAGE_t subscribe_message = mqtt_subscribe_to_pump_command();
  if (subscribe_message != WIFI_OK)
  {
    uart_send_string_blocking(USART_0, "Unable to send subscribe packet!\n");
  }
  else
  {
    uart_send_string_blocking(USART_0, "Sent subscribe packet!\n");
  }

  periodic_task_init_a(loop, 2000);

  while (1)
  {
  }

  return 0;
}