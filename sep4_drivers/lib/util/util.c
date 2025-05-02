/**
 * @brief Function to calculate the moisture percentage from the ADC value
 *
 * This function takes the ADC value from the soil moisture sensor and
 * calculates the moisture percentage based on a linear mapping.
 *
 * @param sensor_value The ADC value from the soil moisture sensor
 * @return The calculated moisture percentage (0-100)
 */ 
int calculate_moisture_percentage(int sensor_value) {
    int moisture_percentage = 100 - ((sensor_value - 200) * 100) / (505 - 200);
    return moisture_percentage;
  }