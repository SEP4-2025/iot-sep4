#include "light_intensity.h"
#include "light.h"
#include <stdio.h>
#include <string.h>

float calculate_light_intensity(uint16_t light_adc)
{
    float voltage = light_adc * (5.0 / 1023.0);
    float resistance = voltage * 10000.0 / (5.0 - voltage);
    float lux = 500.0 / (resistance / 1000.0);

    return lux;
}