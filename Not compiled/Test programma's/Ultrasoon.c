#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serialF0.h"
#include "clock.h"
#include "HCSR04.h"


int main(void) 
{
    float distance;

    init_clock();
    init_stream(F_CPU);
    Init_HCSR04(400, 0);

    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    sei(); 

	while (1) {
        distance = Measure_distance_cm(22.307);
        printf("Distance: %ld cm\n", (uint32_t)(distance));
    }
}