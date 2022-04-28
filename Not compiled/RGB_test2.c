#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serialF0.h"
#include "clock.h"
#include "Kleur_arrays.h"

void init_Line_Follower(void);

int16_t control_Result(volatile int16_t* result);

int main(void) 
{
	init_Line_Follower();
	init_clock();
	init_stream(F_CPU);

	sei(); 

	while (1) 
	{	
    	PORTE_OUTSET = PIN5_bm;
		PORTE_OUTCLR = PIN5_bm;
		PORTE_OUTSET = PIN4_bm;

		_delay_ms(1000);
	}
}


void init_Line_Follower(void)
{
	PORTE_DIRSET = PIN7_bm;		// SIN
	PORTE_DIRCLR = PIN6_bm;		// SOUT
	PORTE_DIRSET = PIN5_bm;		// SCLK
	PORTE_DIRSET = PIN4_bm;		// LAT
	PORTE_DIRSET = PIN3_bm;		// BLANK
	
	PORTE_OUTCLR = PIN7_bm;		// SIN low
	PORTE_OUTSET = PIN3_bm;		// BLANK high
	
	// 96 pulses on SCLK
	for (int i = 0; i < 96; i++) 
	{
		PORTE_OUTSET = PIN5_bm;
		PORTE_OUTCLR = PIN5_bm;
	}
	
	// 1 puls op LAT
	PORTE_OUTSET = PIN4_bm;
	PORTE_OUTCLR = PIN4_bm;

 	PORTE_OUTSET = PIN7_bm;

  	PORTE_OUTSET = PIN5_bm;
	PORTE_OUTCLR = PIN5_bm;

  	PORTE_OUTCLR = PIN7_bm;

  	PORTE_OUTSET = PIN4_bm;
	PORTE_OUTCLR = PIN4_bm;

  	PORTE_OUTCLR = PIN3_bm;		// BLANK low
}


