#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "Ultrasonic.hpp"

extern "C"
{
    #include "serialF0.h"
    #include "clock.h"
}


int main(void) 
{
    float distance_left, distance_right;

    init_clock();
    init_stream(F_CPU);
    
    Ultrasonic left(40, 0, &PORTD, PIN0_bm, PIN1_bm);
    Ultrasonic right(40, 0, &PORTE, PIN0_bm, PIN1_bm);

	while (1) {
        distance_left = (int)left.Measure_distance_cm(22.307);
        distance_right = (int)right.Measure_distance_cm(22.307);
        printf(" %ld %ld\n", (uint32_t)(distance_left), (uint32_t)(distance_right));
        _delay_ms(1);
    }
} 