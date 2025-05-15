#include "sensor_readings.h"
#include "light.h"
#include "soil.h"
#include "mqtt/mqtt.h"
#include "wifi.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include "dht11.h"

int send_soil_moisture_reading(void)
{
    char transmit_buf[200];
    int transmit_buflen = sizeof(transmit_buf);
    char soil_topic[] = "soil:reading";

    uint16_t soil_adc = soil_read();
    int moisture_percentage = calculate_moisture_percentage(soil_adc);

    char soil_payload[100];
    sprintf(soil_payload, "Soil ADC Val: %d\nSoil Moisture: %d%%\n",
            soil_adc, moisture_percentage);

    int transmit_len = create_mqtt_transmit_packet(
        soil_topic, soil_payload, transmit_buf, transmit_buflen);

    return wifi_command_TCP_transmit(transmit_buf, transmit_len);
}

int send_temperature_humidity_reading(void)
{
    char transmit_buf[200];
    int transmit_buflen = sizeof(transmit_buf);
    char dht_topic[] = "dht:reading";

    uint8_t humidity_integer = 0;
    uint8_t humidity_decimal = 0;
    uint8_t temperature_integer = 0;
    uint8_t temperature_decimal = 0;

    DHT11_ERROR_MESSAGE_t dht11_res =
        dht11_get(&humidity_integer, &humidity_decimal, &temperature_integer,
                  &temperature_decimal);

    char dht_payload[100];
    sprintf(dht_payload, "Humidity: %d.%d%%\nTemperature: %d.%d°C\n",
            humidity_integer, humidity_decimal, temperature_integer,
            temperature_decimal);

    int transmit_len = create_mqtt_transmit_packet(
        dht_topic, dht_payload, transmit_buf, transmit_buflen);

    return wifi_command_TCP_transmit(transmit_buf, transmit_len);
}

int send_light_reading(void)
{
    char transmit_buf[200];
    int transmit_buflen = sizeof(transmit_buf);
    char light_topic[] = "light:reading";

    uint16_t light_adc = light_read();

    float lux = calculate_light_intensity(light_adc);
    int lux_int = (int)lux;

    char light_payload[100];
    sprintf(light_payload, "Light ADC Val: %d\nLight Intensity: %d lux\n",
            light_adc, lux_int);

    int transmit_len = create_mqtt_transmit_packet(
        light_topic, light_payload, transmit_buf, transmit_buflen);

    return wifi_command_TCP_transmit(transmit_buf, transmit_len);
}