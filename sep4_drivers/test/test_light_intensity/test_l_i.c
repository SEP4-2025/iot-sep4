#include "unity.h"
#include "light_intensity.h" 

void test_light_intensity_zero_adc(void) {
    float result = calculate_light_intensity(0);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, result); // ADC 0 should be close to 0 lux
}

void test_light_intensity_half_adc(void) {
    float result = calculate_light_intensity(512);
    TEST_ASSERT_FLOAT_WITHIN(0.5, 0.98, result); // Approximate expected lux
}

void test_light_intensity_max_adc(void) {
    float result = calculate_light_intensity(1023);
    // At ADC = 1023, voltage = 5V, so (5 - voltage) = 0 → division by zero risk
    // Your function might return 'inf' or overflow
    TEST_ASSERT_FLOAT_IS_INF(result);
}

void test_light_intensity_typical_adc(void) {
    float result = calculate_light_intensity(300);
    TEST_ASSERT_FLOAT_WITHIN(0.5, 7.0, result); // Expected around 7 lux, depending on formula
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_light_intensity_zero_adc);
    RUN_TEST(test_light_intensity_half_adc);
    RUN_TEST(test_light_intensity_max_adc);
    RUN_TEST(test_light_intensity_typical_adc);
    return UNITY_END();
}

void setUp(void) {
    // Run before each test
}

void tearDown(void) {
    // Run after each test
}
