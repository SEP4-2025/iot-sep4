#pragma once
#include <stdint.h>

// Define pump control pin on port C (connected to Timer 4)
// Using PC7/OC4A (Timer 4 Output Compare A) on J8-2
#define PUMP_PIN PC7

/**
 * Initialize the water pump control
 */
void pump_init(void);

/**
 * Turn the pump on for a specified duration in milliseconds
 * @param duration_ms The duration in milliseconds to run the pump
 * @return 1 if operation started successfully, 0 if pump is already running
 */
uint8_t pump_run(uint32_t duration_ms);

/**
 * Turn off the pump immediately
 */
void pump_stop(void);

/**
 * Check if the pump is currently running
 * @return 1 if pump is running, 0 otherwise
 */
uint8_t pump_is_running(void);

/**
 * Timer interrupt handler to stop the pump after the specified duration
 */
void pump_timer_interrupt_handler(void);
uint8_t pump_start(void);