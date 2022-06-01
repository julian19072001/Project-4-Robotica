#define F_CPU 32000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "serialF0.h"
#include "clock.h"


typedef enum {
	eStateLEDOnMeasureFrontRow, eStateLEDOnMeasureBackRow,
	eStateLEDOffMeasureFrontRow, eStateLEDOffMeasureBackRow,
	eNumStates
} tIntegratorState;


#define INTEGRATION_FREQ			100UL	/* Integrate for one period of the 100Hz-interference of electrical lights. */
#define INTEGRATION_OVERSAMPLING	64UL	/* Number of on-off cycles to integrate in one integration period */

#define PER_ADC						((F_CPU / (INTEGRATION_FREQ * INTEGRATION_OVERSAMPLING * (uint32_t) eNumStates)) - 1)

#define SHFT_BLANK_bm		PIN3_bm
#define SHFT_LATCH_bm		PIN4_bm
#define SHFT_CLK_bm			PIN5_bm
#define SHFT_SOUT_bm		PIN6_bm
#define SHFT_SIN_bm			PIN7_bm

#define BLAUW				1			//Blauw
#define GROEN				2			//Grone
#define ROOD				3			//Rood
#define INFRA_ROOD			4			//Infra Rood

#define LED					4			//Nummer van LED in grid

#define PORTA_ADCPINS		(PIN0_bm|PIN1_bm|PIN2_bm|PIN3_bm|PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm)
#define PORTB_ADCPINS		(PIN0_bm|PIN1_bm|PIN2_bm|PIN3_bm|PIN4_bm|PIN5_bm|PIN6_bm|PIN7_bm)


typedef enum {
	/* Front row */
	eD44, eD41, eD38, eD35, eD32, eD29, eD26,

	/* Back row */
	eD75, eD72, eD69, eD66, eD63, eD60, eD57,

	eNumPhotodiodes
} tPhotoDiodeID;


typedef struct {
	int32_t photodiodes[eNumPhotodiodes];
	int16_t numMeasurements;
} tPhotodiodes;

static volatile bool sRequestNewValues;
static volatile tPhotodiodes sGlobalValues;


static inline void SetADCMux_FrontRow(void) {
	
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN14_gc | ADC_CH_MUXNEG_PIN3_gc; /* D26 */
	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN12_gc | ADC_CH_MUXNEG_PIN3_gc; /* D29 */
	ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN9_gc  | ADC_CH_MUXNEG_PIN3_gc; /* D32 */
	ADCA.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc  | ADC_CH_MUXNEG_PIN3_gc; /* D35 */

	ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN13_gc | ADC_CH_MUXNEG_PIN3_gc; /* D38 */
	ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN10_gc | ADC_CH_MUXNEG_PIN3_gc; /* D41 */
	ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN8_gc  | ADC_CH_MUXNEG_PIN3_gc; /* D44 */
	
} /* SetADCMux_FrontRow */


static inline void SetADCMux_BackRow(void) {
	
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN15_gc | ADC_CH_MUXNEG_PIN3_gc; /* D57 */
	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN13_gc | ADC_CH_MUXNEG_PIN3_gc; /* D60 */
	ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN10_gc | ADC_CH_MUXNEG_PIN3_gc; /* D63 */
	ADCA.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN8_gc  | ADC_CH_MUXNEG_PIN3_gc; /* D66 */

	ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN14_gc | ADC_CH_MUXNEG_PIN3_gc; /* D69 */
	ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN12_gc | ADC_CH_MUXNEG_PIN3_gc; /* D72 */
	ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN9_gc  | ADC_CH_MUXNEG_PIN3_gc; /* D75 */
	
} /* SetADCMux_BackRow */


void init_Functie_ADC (void)
{
	PORTA.DIRCLR     = PORTA_ADCPINS;
	PORTCFG.MPCMASK  = PORTA_ADCPINS;
	PORTA.PIN0CTRL   = PORT_ISC_INPUT_DISABLE_gc;
	
	ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;
	ADCA.CH1.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;
	ADCA.CH2.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;
	ADCA.CH3.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;

	ADCA.CH3.INTCTRL = ADC_CH_INTLVL_LO_gc;								// low level interrupt CH0
	
	ADCA.REFCTRL     = ADC_REFSEL_INT1V_gc;
	ADCA.CTRLB       = ADC_RESOLUTION_12BIT_gc |
					   ADC_CONMODE_bm;
	ADCA.PRESCALER   = ADC_PRESCALER_DIV16_gc;
	ADCA.CTRLA       = ADC_ENABLE_bm;
	ADCA.EVCTRL      = ADC_SWEEP_0123_gc |			   					// sweep CH0, ch1, ch2, ch3
					   ADC_EVSEL_0123_gc |								// select event CH0,1,2,3
					   ADC_EVACT_SYNCSWEEP_gc;							// event triggers ADC
	
	PORTB.DIRCLR     = PORTB_ADCPINS;
	PORTCFG.MPCMASK  = PORTB_ADCPINS;
	PORTB.PIN0CTRL   = PORT_ISC_INPUT_DISABLE_gc;

	ADCB.CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;
	ADCB.CH1.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;
	ADCB.CH2.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;
		
	ADCB.REFCTRL     = ADC_REFSEL_INT1V_gc;
	ADCB.CTRLB       = ADC_RESOLUTION_12BIT_gc |
					   ADC_CONMODE_bm;
	ADCB.PRESCALER   = ADC_PRESCALER_DIV16_gc;
	ADCB.CTRLA       = ADC_ENABLE_bm;
	ADCB.EVCTRL      = ADC_SWEEP_012_gc |			   					// sweep CH0, ch1, ch2
					   ADC_EVSEL_0123_gc |								// select event CH0,1,2,3
					   ADC_EVACT_SYNCSWEEP_gc;								// event triggers ADC
					   
	SetADCMux_FrontRow();
					   
	EVSYS.CH0MUX     = EVSYS_CHMUX_TCE1_OVF_gc;
}

void init_Functie_Klok (void)
{
	
	TCE1.CTRLB = TC_WGMODE_NORMAL_gc;
	TCE1.INTCTRLA = TC_OVFINTLVL_OFF_gc;
	TCE1.CTRLA = TC_CLKSEL_DIV1_gc;
	TCE1.PER = PER_ADC;
	
}

void init_Functie_leddriver (void)
{
	//CLR ALL BEHALVE BLANK
	PORTE.OUTCLR = SHFT_LATCH_bm|SHFT_CLK_bm|SHFT_SOUT_bm|SHFT_SIN_bm;
	//BLANK HOOG
	PORTE.OUTSET = SHFT_BLANK_bm;

	//DIRSET VAN ALLE PINNEN
	PORTE.DIRSET = SHFT_BLANK_bm|SHFT_LATCH_bm|SHFT_CLK_bm|SHFT_SOUT_bm|SHFT_SIN_bm;
	
	for (int i=0; i<96; i++)
	{
		/*
		 Layout #LEDs
		 15	14	13	12	|	11	10	9	8
		 19	18	17	16	|	7	6	5	4
		 23	22	21	20	|	3	2	1	0
		*/
		if (
			   //(LED*##)+COLOR
			 i == (LED*15)+ROOD	|| i == (LED*14)+ROOD	|| i == (LED*13)+ROOD	|| i == (LED*12)+ROOD	|| i == (LED*11)+ROOD	|| i == (LED*10)+ROOD	|| i == (LED*9)+ROOD	|| i == (LED*8)+ROOD		//rij 1
			 //i == (LED*19)+ROOD	|| i == (LED*18)+ROOD	|| i == (LED*17)+ROOD	|| i == (LED*16)+ROOD	|| i == (LED*7)+ROOD	|| i == (LED*6)+ROOD	|| i == (LED*5)+ROOD	|| i == (LED*4)+ROOD	||		//rij 2
			 //i == (LED*23)+ROOD	|| i == (LED*22)+ROOD	|| i == (LED*21)+ROOD	|| i == (LED*20)+ROOD	|| i == (LED*3)+ROOD	|| i == (LED*2)+ROOD	|| i == (LED*1)+ROOD	|| i == (LED*0)+ROOD			//rij 3
			)

			{
				PORTE.OUTSET = SHFT_SIN_bm;
			}
		else
			{
				PORTE.OUTCLR = SHFT_SIN_bm;
			}

		PORTE.OUTSET = SHFT_CLK_bm;

		PORTE.OUTCLR = SHFT_CLK_bm;
	}
	//ÉÉN PULS OP LATCH
	PORTE.OUTSET = SHFT_LATCH_bm;
	PORTE.OUTCLR = SHFT_LATCH_bm;

	//BLANK LAAG
	PORTE.OUTCLR = SHFT_BLANK_bm;
}

ISR (ADCA_CH3_vect){
	
	static tPhotodiodes sLocalValues;
	static tIntegratorState sCurState;
	static uint16_t sOversamplingCount;
	
	switch(sCurState) {
		case eStateLEDOnMeasureBackRow:
			sLocalValues.photodiodes[eD57] += (int16_t) ADCA.CH0.RES;
			sLocalValues.photodiodes[eD60] += (int16_t) ADCA.CH1.RES;
			sLocalValues.photodiodes[eD63] += (int16_t) ADCA.CH2.RES;
			sLocalValues.photodiodes[eD66] += (int16_t) ADCA.CH3.RES;
			sLocalValues.photodiodes[eD69] += (int16_t) ADCB.CH0.RES;
			sLocalValues.photodiodes[eD72] += (int16_t) ADCB.CH1.RES;
			sLocalValues.photodiodes[eD75] += (int16_t) ADCB.CH2.RES;
		
			SetADCMux_FrontRow();
			//BLANK HOOG
			PORTE.OUTSET = SHFT_BLANK_bm;
			sCurState = eStateLEDOffMeasureBackRow;
			break;
		
		case eStateLEDOffMeasureBackRow:
			sLocalValues.photodiodes[eD57] -= (int16_t) ADCA.CH0.RES;
			sLocalValues.photodiodes[eD60] -= (int16_t) ADCA.CH1.RES;
			sLocalValues.photodiodes[eD63] -= (int16_t) ADCA.CH2.RES;
			sLocalValues.photodiodes[eD66] -= (int16_t) ADCA.CH3.RES;
			sLocalValues.photodiodes[eD69] -= (int16_t) ADCB.CH0.RES;
			sLocalValues.photodiodes[eD72] -= (int16_t) ADCB.CH1.RES;
			sLocalValues.photodiodes[eD75] -= (int16_t) ADCB.CH2.RES;
		
			SetADCMux_FrontRow();		
			//BLANK LAAG -- LEDs aan
			PORTE.OUTCLR = SHFT_BLANK_bm;
			sCurState = eStateLEDOnMeasureBackRow;
		
			sLocalValues.numMeasurements++;
			sOversamplingCount++;
			if(sOversamplingCount >= INTEGRATION_OVERSAMPLING) {
				sOversamplingCount = 0;
				if(sRequestNewValues) {
					sGlobalValues = sLocalValues;
					sLocalValues = (const tPhotodiodes) { }; /* reset sLocalValues to all-zero */
					sRequestNewValues = false;
				}
			}
			break;
		default:
			/* Should Not Happen */
			PORTE.OUTCLR = SHFT_BLANK_bm;
			sCurState = eStateLEDOnMeasureBackRow;
			sOversamplingCount = 0;
			sLocalValues = (const tPhotodiodes) { };
	}
}


int main (void)
{
	init_stream(F_CPU);
	init_clock();
	init_Functie_ADC();
	init_Functie_Klok();
	init_Functie_leddriver();
	
	PORTC.DIRSET = PIN0_bm;
	PORTF.DIRSET = PIN0_bm;
	PORTF.DIRSET = PIN1_bm;
	
	printf("Hallo \n");
	
	sei();

	while (1) {
		sRequestNewValues = true;
		while(sRequestNewValues) 
			/* Do nothing */ ;
		//printf("test \n");
		printf(" ");
		for(uint8_t i = 7; i < eNumPhotodiodes; i++) {
			printf(" %d", (int16_t) (sGlobalValues.photodiodes[i] / sGlobalValues.numMeasurements));
		}
		printf("\n");
	
	}
}