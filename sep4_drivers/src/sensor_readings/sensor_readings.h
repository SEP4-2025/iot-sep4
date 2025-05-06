#pragma once

#include <stdint.h>

int calculate_moisture_percentage(int sensor_value);
int send_soil_moisture_reading(void);
float calculate_light_intensity(uint16_t light_adc);
int send_temperature_humidity_reading(void);
int send_light_reading(void);
int send_pump_status(const char* topic, const char* message);