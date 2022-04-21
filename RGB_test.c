#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "libraries/serialF0.h"
#include "libraries/clock.h"

int main(void) {
	init_clock();
	init_stream(F_CPU);
	int led_out_array[96] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							 0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,
							 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							 0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,
							 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	PORTE_DIRSET = PIN7_bm;		// SIN
	PORTE_DIRSET = PIN6_bm;		// SOUT
	PORTE_DIRSET = PIN5_bm;		// SCLK
	PORTE_DIRSET = PIN4_bm;		// LAT
	PORTE_DIRSET = PIN3_bm;		// BLANK
	
	PORTE_OUTCLR = PIN7_bm;		// SIN low
	PORTE_OUTSET = PIN3_bm;		// BLANK high
	
	// 96 pulses on SCLK
	for (int i = 0; i < 96; i++) {
		PORTE_OUTTGL = PIN5_bm;
		PORTE_OUTTGL = PIN5_bm;
	}
	
	// 1 puls op LAT
	PORTE_OUTTGL = PIN4_bm;
	PORTE_OUTTGL = PIN4_bm;
	
	PORTE_OUTCLR = PIN3_bm;		// BLANK low
	
	_delay_ms(500);
	
	PORTE_OUTSET = PIN3_bm;		// BLANK high
	
	////////////////////////////////////////
	
	for (int i = 0; i < 96; i++) {
		if (led_out_array[i] == 1){
			PORTE_OUTSET = PIN7_bm;
		} else {
			PORTE_OUTCLR = PIN7_bm;
		}
		
		PORTE_OUTTGL = PIN5_bm;
		PORTE_OUTTGL = PIN5_bm;
	}
	
	// 1 puls op LAT
	PORTE_OUTTGL = PIN4_bm;
	PORTE_OUTTGL = PIN4_bm;
	
	PORTE_OUTCLR = PIN3_bm;		// BLANK low

	PORTA.DIRCLR     = PIN0_bm;															//configure PA0 as input for ADCA
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_GND_MODE3_gc;				//PA0 to channel 0
	ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;										//differential, no gain

	ADCA.CTRLB       =	ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm | ADC_FREERUN_bm; 		//free running mode

	ADCA.REFCTRL     =  ADC_REFSEL_INTVCC_gc;											//internal vcc/1.6 refernce
	ADCA.PRESCALER   =  ADC_PRESCALER_DIV16_gc;											//prescaling
	ADCA.CTRLA       =  ADC_ENABLE_bm;													//enable ADC

	sei(); 

	PMIC.CTRL |= PMIC_LOLVLEN_bm;

	int val;
	while (1) {
	ADCA.CH0.CTRL |= ADC_CH_START_bm;                  						//start ADC conversion
	while ( !(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm) ) ;   					//wait until it's ready
	val = ADCA.CH0.RES;														//store the ADC value
	ADCA.CH0.INTFLAGS |= ADC_CH_CHIF_bm;                					//reset interrupt flag
	printf("%d", val);
	}
}

