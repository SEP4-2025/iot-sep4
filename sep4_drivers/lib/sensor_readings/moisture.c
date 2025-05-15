#include "moisture.h"
#include <stdio.h>
#include <string.h>

int calculate_moisture_percentage(int sensor_value)
{
    int moisture_percentage = 100 - ((sensor_value - 200) * 100) / (505 - 200);

    if (moisture_percentage > 100)
    {
        moisture_percentage = 100;
    }
    else if (moisture_percentage < 0)
    {
        moisture_percentage = 0;
    }

    return moisture_percentage;
}