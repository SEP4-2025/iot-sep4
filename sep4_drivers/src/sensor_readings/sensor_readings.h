#pragma once

#include <stdint.h>

int send_soil_moisture_reading(void);
int send_temperature_humidity_reading(void);
int send_light_reading(void);
int send_pump_status(const char* topic, const char* message);