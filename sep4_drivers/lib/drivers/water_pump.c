#include "water_pump.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// Flag to track if pump is currently running
static volatile uint8_t pump_running = 0;
// Counter for tracking pump duration
static volatile uint32_t pump_duration_counter = 0;

// Initialize Timer 4 for pump timing using CTC mode
void pump_init(void)
{
    // Configure PUMP_PIN as output
    DDRH |= (1 << PUMP_PIN);
    
    // Ensure pump is off initially
    PORTH &= ~(1 << PUMP_PIN);
    
    // Configure Timer 4
    // CTC mode (Clear Timer on Compare match)
    TCCR4A = 0;
    TCCR4B = (1 << WGM42); // CTC mode, OCR4A as TOP
    
    // Set compare value for approximately 1ms intervals (16MHz/64/250 = 1ms)
    OCR4A = 249;
    
    // Enable Timer 4 compare A match interrupt
    TIMSK4 = (1 << OCIE4A);
    
    pump_running = 0;
}

uint8_t pump_run(uint32_t duration_ms)
{
    // Check if pump is already running
    if (pump_running) {
        return 0;
    }
    
    // Disable interrupts while configuring
    cli();
    
    // Set duration counter (in milliseconds)
    pump_duration_counter = duration_ms;
    
    // Turn on pump
    PORTH |= (1 << PUMP_PIN);
    
    // Set flag to indicate pump is running
    pump_running = 1;
    
    // Start timer with 64 prescaler
    // CS42:40 = 0b011 for prescaler 64
    TCCR4B |= (1 << CS41) | (1 << CS40);
    
    // Re-enable interrupts
    sei();
    
    return 1;
}

void pump_stop(void)
{
    // Stop timer
    TCCR4B &= ~((1 << CS42) | (1 << CS41) | (1 << CS40));
    
    // Turn off pump
    PORTH &= ~(1 << PUMP_PIN);
    
    // Clear running flag
    pump_running = 0;
}

uint8_t pump_is_running(void)
{
    return pump_running;
}

// Timer 4 Compare A Match Interrupt Service Routine
ISR(TIMER4_COMPA_vect)
{
    if (pump_running) {
        if (pump_duration_counter > 0) {
            pump_duration_counter--;
        }
        
        if (pump_duration_counter == 0) {
            // Time is up, stop the pump
            pump_stop();
        }
    }
}