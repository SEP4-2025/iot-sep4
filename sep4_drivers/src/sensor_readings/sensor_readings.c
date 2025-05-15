#include "sensor_readings.h"
#include "dht11.h"
#include "light.h"
#include "mqtt/mqtt.h"
#include "soil.h"
#include "uart.h"
#include "wifi.h"
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>

char log_buf[250];

int calculate_moisture_percentage(int sensor_value) {
  int moisture_percentage = 100 - ((sensor_value - 200) * 100) / (505 - 200);

  if (moisture_percentage > 100) {
    moisture_percentage = 100;
  } else if (moisture_percentage < 0) {
    moisture_percentage = 0;
  }

  return moisture_percentage;
}

float calculate_light_intensity(uint16_t light_adc) {
  float voltage = light_adc * (5.0 / 1023.0);
  float resistance = voltage * 10000.0 / (5.0 - voltage);
  float lux = 500.0 / (resistance / 1000.0);

  return lux;
}

int send_soil_moisture_reading(void) {
  char transmit_buf[100];
  int transmit_buflen = sizeof(transmit_buf);
  char soil_topic[] = "soil/reading";

  uint16_t soil_adc = soil_read();
  int moisture_percentage = calculate_moisture_percentage(soil_adc);

  char soil_payload[100];
  sprintf(soil_payload, "%d", moisture_percentage);

  sprintf(log_buf, "Soil humidity: %d\n\%", moisture_percentage);
  uart_send_string_blocking(USART_0, log_buf);

  int transmit_len = create_mqtt_transmit_packet(soil_topic, soil_payload,
                                                 transmit_buf, transmit_buflen);

  return wifi_command_TCP_transmit(transmit_buf, transmit_len);
}

// int send_temperature_humidity_reading(void) {
//   char transmit_temp_buf[100];
//   int transmit_temp_buflen = sizeof(transmit_temp_buf);
//   char dht_topic[] = "dht/reading";

//   uint8_t humidity_integer = 0;
//   uint8_t humidity_decimal = 0;
//   uint8_t temperature_integer = 0;
//   uint8_t temperature_decimal = 0;

//   dht11_get(&humidity_integer, &humidity_decimal, &temperature_integer,
//             &temperature_decimal);

//   char dht_payload[100];
//   sprintf(dht_payload, "%d.%d,%d.%d", humidity_integer, humidity_decimal,
//           temperature_integer, temperature_decimal);

//   int transmit_len = create_mqtt_transmit_packet(
//       dht_topic, dht_payload, transmit_temp_buf, transmit_temp_buflen);

//   return wifi_command_TCP_transmit(transmit_temp_buf, transmit_len);
// }

int send_temperature_humidity_reading(void) {
    char transmit_buf[100];
    int transmit_buflen = sizeof(transmit_buf);

    uint8_t humidity_integer = 0;
    uint8_t humidity_decimal = 0;
    uint8_t temperature_integer = 0;
    uint8_t temperature_decimal = 0;

    dht11_get(&humidity_integer, &humidity_decimal, &temperature_integer, &temperature_decimal);

    char temperature_topic[] = "air/temperature";
    char temperature_payload[50];

    sprintf(log_buf, "Air temperature: %d\n", temperature_integer);
    uart_send_string_blocking(USART_0, log_buf);

    // sprintf(temperature_payload, "%d.%d", temperature_integer, temperature_decimal);
    sprintf(temperature_payload, "%d", temperature_integer);

    int temperature_packet_len = create_mqtt_transmit_packet(
        temperature_topic, temperature_payload, transmit_buf, transmit_buflen);

    int temp_result = wifi_command_TCP_transmit(transmit_buf, temperature_packet_len);

    _delay_ms(500);

    char humidity_topic[] = "air/humidity";
    char humidity_payload[50];
    sprintf(humidity_payload, "%d", humidity_integer);

    sprintf(log_buf, "Air humidity: %d\n", humidity_integer);
    uart_send_string_blocking(USART_0, log_buf);

    int humidity_packet_len = create_mqtt_transmit_packet(
        humidity_topic, humidity_payload, transmit_buf, transmit_buflen);

    int humidity_result = wifi_command_TCP_transmit(transmit_buf, humidity_packet_len);

    return (temp_result == 0 && humidity_result == 0) ? 0 : -1;
}


int send_light_reading(void) {
  char transmit_buf[100];
  int transmit_buflen = sizeof(transmit_buf);
  char light_topic[] = "light/reading";

  uint16_t light_adc = light_read();

  float lux = calculate_light_intensity(light_adc);
  int lux_int = (int)lux;

  char light_payload[100];
  sprintf(light_payload, "%d", lux_int);

  int transmit_len = create_mqtt_transmit_packet(light_topic, light_payload,
                                                 transmit_buf, transmit_buflen);

  sprintf(log_buf, "Light readings: %d\n", lux_int);
  uart_send_string_blocking(USART_0, log_buf);

  return wifi_command_TCP_transmit(transmit_buf, transmit_len);
}

int send_pump_status(const char *topic, const char *message) {
  char transmit_buf[200];
  int transmit_buflen = sizeof(transmit_buf);
  char payload[150];
  sprintf(payload, message);

  int transmit_len = create_mqtt_transmit_packet(topic, payload, transmit_buf,
                                                 transmit_buflen);

  return wifi_command_TCP_transmit(transmit_buf, transmit_len);
}
