#include "unity.h"
#include "util.h"  

void test_moisture_low_value(void) {
    TEST_ASSERT_EQUAL_INT(100, calculate_moisture_percentage(200)); // Minimum sensor value
}

void test_moisture_high_value(void) {
    TEST_ASSERT_EQUAL_INT(0, calculate_moisture_percentage(505)); // Maximum sensor value
}

void test_moisture_mid_value(void) {
    TEST_ASSERT_EQUAL_INT(51, calculate_moisture_percentage(352)); // Midpoint
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_moisture_low_value);
    RUN_TEST(test_moisture_high_value);
    RUN_TEST(test_moisture_mid_value);
    return UNITY_END();
}
void setUp(void)
{
    // This function is called before each test
}

void tearDown(void)
{
    // This function is called after each test
}

