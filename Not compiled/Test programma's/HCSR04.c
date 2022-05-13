#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "HCSR04.h"

static uint16_t max_distance_cm;
static uint64_t max_timeout_us;
volatile uint64_t time_us;

static void Init_HCSR04_timer(void);
static void Stop_HCSR04_timer(void);

void Init_HCSR04(uint16_t max_dis, uint64_t max_timeout) {
    max_distance_cm = max_dis;
    max_timeout_us = max_timeout;
    time_us = 0;
    PORTD.DIRCLR = PIN0_bm;		// ECHO
	PORTD.DIRSET = PIN1_bm;		// TRIG
}

void Init_HCSR04_timer(void) {
    TCE1.PER      = 31;     				    // Tper =  1 * (31 + 1) / 32M = 0.000001 s
    TCE1.CTRLA    = TC_CLKSEL_DIV1_gc;          // Prescaling 1
    TCE1.CTRLB    = TC_WGMODE_NORMAL_gc;        // Normal mode
    TCE1.INTCTRLA = TC_OVFINTLVL_LO_gc;        	// Interrupt overflow off
}

void Stop_HCSR04_timer(void) {
	TCE1.CTRLA    = TC_CLKSEL_OFF_gc;			// timer/counter off
    TCE1.INTCTRLA = TC_OVFINTLVL_OFF_gc;		// disables overflow interrupt
}

float Measure_distance_cm(float temperature) {
    uint64_t max_distance_duration_us;
    uint64_t duration_us;

    // Define speed of sound in cm per second
    float speed_of_sound_cm_per_us = 0.03313 + 0.0000606 * temperature; // Cair ≈ (331.3 + 0.606 ⋅ ϑ) m/s

    // Compute max delay based on max distance with 25% margin in microseconds
    max_distance_duration_us = 2.5 * (float)max_distance_cm / speed_of_sound_cm_per_us;
    if (max_timeout_us > 0) {
        if (max_distance_duration_us < max_timeout_us){
            max_distance_duration_us = max_timeout_us;
        }
    }

    PORTD_OUTCLR = PIN1_bm;		// TRIG low
    _delay_us(40);

    // Hold trigger for 10 microseconds, which is signal for sensor to measure distance.
    PORTD_OUTSET = PIN1_bm;		// TRIG high
    _delay_us(2000);
    PORTD_OUTCLR = PIN1_bm;		// TRIG low

    // Measure the length of echo signal, which is equal to the time needed for sound to go there and back.
    time_us = 0;
    Init_HCSR04_timer();
    //loop_until_bit_is_set(PORTD.IN, PIN0_bp);
    while((bit_is_clear(PORTD.IN, PIN0_bp)) && (time_us < max_distance_duration_us));
    time_us = 0;
    while((bit_is_set(PORTD.IN, PIN0_bp)) && (time_us < max_distance_duration_us));
    Stop_HCSR04_timer();

    if (time_us < max_distance_duration_us) {
        duration_us = time_us;
    } else {
        duration_us = 0;
    }
    
    float distance_cm = duration_us / 2.0 * speed_of_sound_cm_per_us * 3.6;

    if (distance_cm == 0 || distance_cm > max_distance_cm) {
        return 0 ;
    } else {
        return distance_cm;
    }
}

ISR (TCE1_OVF_vect) {
    time_us++;
}