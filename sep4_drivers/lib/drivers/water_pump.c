#include "water_pump.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// Define pump control pin on port C (connected to Timer 4)
// Using PC7/OC4A (Timer 4 Output Compare A) on J8-2
#define PUMP_PIN PC7

// Timer 4 configuration
volatile uint32_t pump_duration = 0;  // Duration to run the pump
volatile uint8_t pump_running = 0;    // Flag to track pump status

/**
 * Initialize the water pump control
 */
void pump_init(void) {
    // Set PC7 as output for pump control
    DDRC |= (1 << PUMP_PIN);
    // Make sure pump starts in OFF state
    PORTC &= ~(1 << PUMP_PIN);

    // Set up Timer 4 for a 1 ms interrupt
    // Reset the timer control registers first
    TCCR4A = 0;
    TCCR4B = 0;
    TCNT4 = 0;  // Initialize counter value to 0
    
    // Set CTC mode (Clear Timer on Compare Match)
    TCCR4B |= (1 << WGM42);
    
    // Set prescaler to 64 for more reliable timing
    TCCR4B |= (1 << CS41) | (1 << CS40);  // 64 prescaler
    
    // Calculate the compare match value
    // 16MHz / 64 = 250kHz timer frequency
    // 250kHz / 250 = 1000Hz = 1ms period
    OCR4A = 249;
    
    // Enable Timer 4 Compare A interrupt
    TIMSK4 |= (1 << OCIE4A);
    
    // Enable global interrupts
    sei();
}

/**
 * Turn the pump on for a specified duration in milliseconds
 * @param duration_ms The duration in milliseconds to run the pump
 * @return 1 if operation started successfully, 0 if pump is already running
 */
uint8_t pump_run(uint32_t duration_ms) {
    cli();  // Disable interrupts while updating volatile variables
    
    // Check if pump is already running
    if (pump_running) {
        sei();  // Re-enable interrupts
        return 0;
    }

    // Set the pump duration (starting the countdown)
    pump_duration = duration_ms;
    pump_running = 1;  // Mark pump as running
    
    // Set PC7 high to turn on the pump
    PORTC |= (1 << PUMP_PIN);
    send_pump_status("pump:started", "pump started");
    
    sei();  // Re-enable interrupts
    return 1;
}

/**
 * Turn off the pump immediately
 */
void pump_stop(void) {
    cli();  // Disable interrupts briefly
    
    // Set PC7 low to turn off the pump
    PORTC &= ~(1 << PUMP_PIN);

    // Reset duration and running flag
    pump_duration = 0;
    pump_running = 0;
    
    sei();  // Re-enable interrupts
}

/**
 * Check if the pump is currently running
 * @return 1 if pump is running, 0 otherwise
 */
uint8_t pump_is_running(void) {
    return pump_running;  // Return whether the pump is running
}

/**
 * Timer interrupt handler to stop the pump after the specified duration
 */
ISR(TIMER4_COMPA_vect) {
    if (pump_running && pump_duration > 0) {
        pump_duration--;  // Decrement the duration
        
        // If the duration reaches 0, stop the pump
        if (pump_duration == 0) {
            // Set PC7 low to turn off the pump
            PORTC &= ~(1 << PUMP_PIN);
            pump_running = 0;  // Mark pump as stopped
            send_pump_status("pump:stopped", "pump stopped");
        }
    }
}