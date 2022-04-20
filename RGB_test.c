#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
	
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
	
	while (1) {}
}

