#include "display.h"
#include "light.h"
#include "mqtt.h"
#include "periodic_task.h"
#include "servo.h"
#include "soil.h"
#include "uart.h"
#include "wifi.h"
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include "MQTTPacket.h"
#include <string.h>

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

// static void my_event_cb(const void *evt, void *data) {
//   /* do stuff and things with the event */
//   uart_send_string_blocking(USART_0, "Hello from callback!\n");
// }

void my_event_cb()
{
    // _buff points to the received message
    unsigned char* buf = (unsigned char*)_buff;
    int buflen = strlen(_buff);  // or keep track separately if binary data
    unsigned char packet_type = buf[0] >> 4;
    uart_send_string_blocking(USART_0, _buff);

    switch (packet_type)
    {
        case CONNACK:
        {
            unsigned char sessionPresent, connack_rc;
            char msg_buf[100];
            if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) == 1) {
                sprintf(msg_buf, "MQTT: CONNACK received, sessionPresent=%d, returnCode=%d\n", sessionPresent, connack_rc);
                uart_send_string_blocking(USART_0, msg_buf);
            } else {
                uart_send_string_blocking(USART_0, "MQTT: Failed to deserialize CONNACK\n");
            }
            break;
        }

        case SUBACK:
        {
            unsigned short packetid;
            int count;
            int grantedQoSs[10];
            char msg_buf[100];

            if (MQTTDeserialize_suback(&packetid, 10, &count, grantedQoSs, buf, buflen) == 1) {
                sprintf(msg_buf, "MQTT: SUBACK received for packetId=%d with %d QoS entries\n", packetid, count);
                uart_send_string_blocking(USART_0, msg_buf);

                for (int i = 0; i < count; ++i) {
                    sprintf(msg_buf, "  Granted QoS: %d\n", grantedQoSs[i]);
                    uart_send_string_blocking(USART_0, msg_buf);
                }
            } else {
                uart_send_string_blocking(USART_0, "MQTT: Failed to deserialize SUBACK\n");
            }
            break;
        }

        case PUBLISH:
        {
            unsigned char dup, retained;
            int qos, payloadlen;
            unsigned short packetid;
            MQTTString topicName;
            unsigned char* payload;
            char msg_buf[200];

            if (MQTTDeserialize_publish(&dup, &qos, &retained, &packetid,
                                        &topicName, &payload, &payloadlen, buf, buflen) == 1) {
                sprintf(msg_buf, "MQTT: PUBLISH received on topic ");
                uart_send_string_blocking(USART_0, msg_buf);

                // Send topic
                for (int i = 0; i < topicName.lenstring.len; ++i)
                uart_send_blocking(USART_0, topicName.lenstring.data[i]);

                uart_send_string_blocking(USART_0, ": ");

                // Send payload
                for (int i = 0; i < payloadlen; ++i)
                    uart_send_blocking(USART_0, payload[i]);

                uart_send_string_blocking(USART_0, "\n");

            } else {
                uart_send_string_blocking(USART_0, "MQTT: Failed to deserialize PUBLISH\n");
            }
            break;
        }

        default:
        {
            char msg_buf[50];
            sprintf(msg_buf, "MQTT: Unknown packet type: %d\n", packet_type);
            uart_send_string_blocking(USART_0, msg_buf);
            break;
        }
    }
}


/**
 * @brief Function to calculate the moisture percentage from the ADC value
 *
 * This function takes the ADC value from the soil moisture sensor and
 * calculates the moisture percentage based on a linear mapping.
 *
 * @param sensor_value The ADC value from the soil moisture sensor
 * @return The calculated moisture percentage (0-100)
 */
int calculate_moisture_percentage(int sensor_value)
{
  int moisture_percentage = 100 - ((sensor_value - 200) * 100) / (505 - 200);
  return moisture_percentage;
}

// Function to create and serialize the MQTT connect packet
void loop()
{
  char light_transmit_buf[200];
  int light_transmit_buflen = sizeof(light_transmit_buf);
  char light_topic[] = "light:reading";
  uint16_t light_adc = light_read();
  int moisture_percentage = calculate_moisture_percentage(light_adc);
  /* float vol = light_adc * (5 / 1023.0); */
  /* float res = vol * 10000.0 / (5 - vol); */
  /* float lux = 500.0 / (res / 1000.0); */
  /* int lux_int = (int)lux; */
  char light_payload[100] = "";
  sprintf(light_payload, "Soil ADC Val: %d\n Soil data:%d\n", light_adc,
          moisture_percentage);
  int transmit_len = create_mqtt_transmit_packet(
      light_topic, light_payload, light_transmit_buf, light_transmit_buflen);
  wifi_command_TCP_transmit(light_transmit_buf, transmit_len);
  /**/
  /* uint8_t humidity_integer = 0; */
  /* uint8_t humidity_decimal = 0; */
  /* uint8_t temperature_integer = 0; */
  /* uint8_t temperature_decimal = 0; */
  /* DHT11_ERROR_MESSAGE_t dht11_res = */
  /*     dht11_get(&humidity_integer, &humidity_decimal, &temperature_integer,
   */
  /**/
  /*               &temperature_decimal); */
  /* char dht_payload[100] = ""; */
  /* sprintf(dht_payload, "Humidity: %d.%d, Temperature: %d.%d\n", */
  /*         humidity_integer, humidity_decimal, temperature_integer, */
  /*         temperature_decimal); */
  /* int transmit_len = create_mqtt_transmit_packet(topic, dht_payload, */
  /*                                                transmit_buf,
   * transmit_buflen); */
}

// void receive_loop()
// {

//   uart_send_string_blocking(USART_0, "Start pump!\n");
//   uint8_t buffer[256];
//   uint16_t receivedLength = 0;
//   unsigned char dup, retained;
//   int qos;
//   unsigned short packetId;
//   MQTTString topicName;
//   unsigned char *payload;
//   int payloadLen;

//   WIFI_ERROR_MESSAGE_t error = wifi_command_TCP_receive(buffer, sizeof(buffer), &receivedLength);
//   if (error != WIFI_OK)
//     return;

//   int rc = MQTTDeserialize_publish(&dup, &qos, &retained, &packetId, &topicName, &payload, &payloadLen, buffer, receivedLength);
//   if (rc != 1)
//     return;
// }

WIFI_ERROR_MESSAGE_t mqtt_subscribe_to_pump_command()
{
  uart_send_string_blocking(USART_0, "mqqt subscribe to pump command start!\n");
  uint8_t buffer[128];
  MQTTString topic = MQTTString_initializer;
  topic.cstring = "pump:command";
  uint16_t packetId = 1; // Can be incremented if you send multiple subscriptions
  int qos = 1;           // QoS level 1 (as expected)

  int len = MQTTSerialize_subscribe(buffer, sizeof(buffer), 0, packetId, 1, &topic, &qos);
  if (len <= 0)
    return WIFI_FAIL;

  uart_send_string_blocking(USART_0, "mqqt subscribe to pump command end!\n");
  // Send the subscribe packet over TCP
  return wifi_command_TCP_transmit(buffer, len);
}

// WIFI_ERROR_MESSAGE_t mqtt_suback_receive()
// {
//   uint8_t buffer[64];
//   unsigned short packetId;
//   int count;
//   int grantedQoS[1]; // We only subscribe to one topic
//   int rc;

//   // Receive SUBACK from the server
//   WIFI_ERROR_MESSAGE_t error = wifi_command_TCP_receive(buffer, sizeof(buffer));
//   if (error != WIFI_OK)
//     return error;

//   // Deserialize the SUBACK packet
//   rc = MQTTDeserialize_suback(&packetId, 1, &count, grantedQoS, buffer, sizeof(buffer));
//   if (rc != 1)
//     return WIFI_FAIL;

//   // Optionally check if QoS is granted
//   if (grantedQoS[0] == 0x80) // 0x80 = failure in MQTT
//     return WIFI_FAIL;

//   return WIFI_OK;
// }

int main()
{
  // Init wifi and light
  wifi_init();
  light_init();
  soil_init();

  // Writing in the console
  uart_init(USART_0, 9600, console_rx);
  uart_send_string_blocking(USART_0, "Hello from main!\n");

  // Connect to wifi network
  WIFI_ERROR_MESSAGE_t wifi_res =
      wifi_command_join_AP("Marius iPhone", "pidaras69");

  // Connect to TCP server
  // Write callback function to type in the messag ein the uart
  char *_buff = malloc(100);
  wifi_command_create_TCP_connection("172.20.10.4", 1883, my_event_cb,
                                     _buff);

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
  unsigned char connect_buf[200];
  int connect_buflen = sizeof(connect_buf);
  int connect_len = create_mqtt_connect_packet(connect_buf, connect_buflen);
  if (connect_len > 0)
  {
    uart_send_string_blocking(USART_0, "MQTT Connect packet created!\n");
  }
  wifi_command_TCP_transmit(connect_buf, connect_len);

  _delay_ms(5000);
  WIFI_ERROR_MESSAGE_t subscribe_message = mqtt_subscribe_to_pump_command();

  if (subscribe_message != WIFI_OK)
  {
    uart_send_string_blocking(USART_0, "Unable to send subscribe packet!\n");
  }
  else
  {
    uart_send_string_blocking(USART_0, "Sent subscribe packet!\n");
  }

  // subscribe_message = mqtt_suback_receive();

  // if (subscribe_message != WIFI_OK)
  // {
  //   uart_send_string_blocking(USART_0, "Unable to receive suback packet!\n");
  // }
  // else {
  //   uart_send_string_blocking(USART_0, "Received suback!\n");
  // }

  periodic_task_init_a(loop, 2000);
  //   periodic_task_init_a(receive_loop, 2000);
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
