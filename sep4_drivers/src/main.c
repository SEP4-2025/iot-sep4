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
#include "water_pump.h"

static uint8_t _buff[100];
// static char callback_buff[256];
static unsigned char callback_buff[256];
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

void my_event_cb()
{
    // Check if we have valid data
    if (callback_buff[0] == 0) {
        uart_send_string_blocking(USART_0, "Empty buffer received\n");
        return;
    }

    // Extract MQTT packet type from first byte
    unsigned char packet_type = (callback_buff[0] >> 4) & 0x0F;
    
    // Print packet type in a meaningful way
    char msg_buf[250];
    sprintf(msg_buf, "MQTT packet received - Type: %d\n", packet_type);
    uart_send_string_blocking(USART_0, msg_buf);
  
    
    // Process specific MQTT packet types
    switch (packet_type) {
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
                unsigned char* payload = NULL;
                int payloadlen = 0;
                
                int result = MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, 
                                                  &topicName, &payload, &payloadlen, 
                                                  callback_buff, 256);
                
                if (result == 1) {
                    // Create a null-terminated string for the topic
                    char topic[64] = {0};
                    if (topicName.lenstring.len < sizeof(topic)) {
                        memcpy(topic, topicName.lenstring.data, topicName.lenstring.len);
                        topic[topicName.lenstring.len] = '\0';
                    }
                    
                    // Create a null-terminated string for the payload
                    char payload_str[128] = {0};
                    if (payloadlen < sizeof(payload_str)) {
                        memcpy(payload_str, payload, payloadlen);
                        payload_str[payloadlen] = '\0';
                    }
                    
                    sprintf(msg_buf, "PUBLISH: Topic='%s', Payload='%s', QoS=%d\n", 
                            topic, payload_str, qos);
                    uart_send_string_blocking(USART_0, msg_buf);
                    
                    // Check if this is the pump command topic
                    if (strcmp(topic, "pump:command") == 0) {
                        if (strcmp(payload_str, "start") == 0) {
                            uart_send_string_blocking(USART_0, "Command received: Turn pump ON\n");

                            // Add code to turn on the pump here
                        }
                    }
                } else {
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

WIFI_ERROR_MESSAGE_t mqtt_subscribe_to_pump_command()
{
  uint8_t buffer[128];
  MQTTString topic = MQTTString_initializer;
  topic.cstring = "pump:command";
  uint16_t packetId = 1; // Can be incremented if you send multiple subscriptions
  int qos = 1;           // QoS level 1 (as expected)

  int len = MQTTSerialize_subscribe(buffer, sizeof(buffer), 0, packetId, 1, &topic, &qos);
  if (len <= 0)
    return WIFI_FAIL;

  // Send the subscribe packet over TCP
  return wifi_command_TCP_transmit(buffer, len);
}


// int main()
// {
//   // Init wifi and light
//   wifi_init();
//   light_init();
//   soil_init();

//   // Writing in the console
//   uart_init(USART_0, 9600, console_rx);
//   uart_send_string_blocking(USART_0, "Hello from main!\n");

//   // Connect to wifi network
//   WIFI_ERROR_MESSAGE_t wifi_res =
//       wifi_command_join_AP("Marius iPhone", "password123");

//   // Connect to TCP server
//   // Write callback function to type in the messag ein the uart
//   // callback_buff = malloc(100);
//   wifi_command_create_TCP_connection("172.20.10.4", 1883, my_event_cb,
//                                      callback_buff);

//   // Log the result of the wifi connection
//   char wifi_res_msg[128];
//   sprintf(wifi_res_msg, "Error: %d \n", wifi_res);
//   uart_send_string_blocking(USART_0, wifi_res_msg);

//   if (wifi_res != WIFI_OK)
//   {
//     uart_send_string_blocking(USART_0, "Error connecting to wifi!\n");
//     return -1;
//   }
//   else
//   {
//     uart_send_string_blocking(USART_0, "Connected to wifi!\n");
//   }
//   unsigned char connect_buf[200];
//   int connect_buflen = sizeof(connect_buf);
//   int connect_len = create_mqtt_connect_packet(connect_buf, connect_buflen);
//   if (connect_len > 0)
//   {
//     uart_send_string_blocking(USART_0, "MQTT Connect packet created!\n");
//   }
//   wifi_command_TCP_transmit(connect_buf, connect_len);

//   _delay_ms(5000);
//   WIFI_ERROR_MESSAGE_t subscribe_message = mqtt_subscribe_to_pump_command();

//   if (subscribe_message != WIFI_OK)
//   {
//     uart_send_string_blocking(USART_0, "Unable to send subscribe packet!\n");
//   }
//   else
//   {
//     uart_send_string_blocking(USART_0, "Sent subscribe packet!\n");
//   }


//   periodic_task_init_a(loop, 2000);
//   while (1)
//   {
//   }
//   // char disconnect_buf[200];
//   // int disconnect_buflen = sizeof(disconnect_buf);
//   // int disconnect_len =
//   //     create_mqtt_disconnect_packet(disconnect_buf, disconnect_buflen);
//   // if (disconnect_len > 0)
//   // {
//   //     printf("MQTT Disconnect packet created. Length: %d\n",
//   //            disconnect_len);
//   // }
//   return 0;
// }

int main() {
  pump_init();
    
  sei();
    
  pump_run(5000);
    
  while(1) {
    
  }
    
  return 0;
}