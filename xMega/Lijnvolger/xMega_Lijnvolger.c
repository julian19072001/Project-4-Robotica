#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serialF0.h"
#include "clock.h"
#include "spi.h"
//#include "Kleur_arrays.h"

#define SAMPLES 8                     //number of samples used to create an average
#define SAMPLE_DELAY 1                //time in milliseconds between samples

#define SHFT_LATCH_bm   PIN4_bm 

// Defines for led control
#define RED     0x11
#define GREEN   0x22
#define BLUE    0x44
#define WHITE   0x77
#define BLACK   0x00
#define IR      0x88

typedef struct measurements
{
  volatile int d75;
  volatile int d72;
  volatile int d69;
  volatile int d66;
  volatile int d63;
  volatile int d60;
  volatile int d57;
}
measurements_t;

static measurements_t result;
static measurements_t zero;

volatile uint16_t reset_d75, reset_d72, reset_d69, reset_d66, reset_d63, reset_d60, reset_d57;
volatile uint8_t sample_count;

void init_Line_Follower(void);
void init_Timer(void);
void stop_Timer(void);
void leds_reset(void);
void leds_front(uint8_t color);
void leds_latch_data(void);
void get_Zero(void);

int16_t control_Result(volatile int16_t* result);

int main(void) 
{
  uint8_t command;

	//init_Timer();
  init_clock();
  spi_init();
	init_stream(F_CPU);

  PORTE_DIRSET = PIN4_bm;		// LAT
	PORTE_DIRSET = PIN3_bm;		// BLANK
  
  init_Line_Follower();

  leds_front(RED);
	
  sei();

	while (1) 
	{	
    measurements_t final;

    //leds_front(BLACK);
    PORTE.OUTCLR = SPI_SS_bm;
    init_Timer();
    while (sample_count < 2) {}
    zero.d75 = control_Result(&result.d75);
    zero.d72 = control_Result(&result.d72);
    zero.d69 = control_Result(&result.d69);
    zero.d66 = control_Result(&result.d66);
    zero.d63 = control_Result(&result.d63);
    zero.d60 = control_Result(&result.d60);
    zero.d57 = control_Result(&result.d57);
    stop_Timer();
    _delay_us(800);

    //leds_front(RED);
    PORTE.OUTSET = SPI_SS_bm;
    init_Timer();
    while (sample_count < 2) {}
    final.d75 = control_Result(&result.d75) - zero.d75;
    final.d72 = control_Result(&result.d72) - zero.d72;
    final.d69 = control_Result(&result.d69) - zero.d69;
    final.d66 = control_Result(&result.d66) - zero.d66;
    final.d63 = control_Result(&result.d63) - zero.d63;
    final.d60 = control_Result(&result.d60) - zero.d60;
    final.d57 = control_Result(&result.d57) - zero.d57;
    stop_Timer();
    _delay_us(8+00);

    printf(" %d %d %d %d %d %d %d\n", final.d75, final.d72, final.d69, final.d66, final.d63, final.d60, final.d57);
	}
}

//Interrupt service routine for d75
ISR(ADCA_CH0_vect)
{
  static uint8_t n_d75 = 0;
  static int16_t sum_d75 = 0;

  if(reset_d75 == 1){
    n_d75 = 0;
    sum_d75 = 0;
    reset_d75 = 0;
  }

  if(n_d75 & 0x01) 
  {                  		    //second (even) measurement
    sum_d75 -= (ADCA.CH0.RES);
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_d75 += (ADCA.CH0.RES);
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN1_gc;
  }

  n_d75++;
  if(n_d75 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    result.d75 = sum_d75/SAMPLES;
    sum_d75 = 0;
    n_d75 = 0;
    sample_count++;
  }
}

//Interrupt service routine for d72
ISR(ADCA_CH1_vect)
{
  static uint8_t n_d72 = 0;
  static int16_t sum_d72 = 0;

  if(reset_d72 == 1){
    n_d72 = 0;
    sum_d72 = 0;
    reset_d72 = 0;
  }

  if(n_d72 & 0x01) 
  {                  		//second (even) measurement
    sum_d72 -= (ADCA.CH1.RES);
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN4_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_d72 += (ADCA.CH1.RES);
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN4_gc;
  }

  n_d72++;
  if(n_d72 == SAMPLES)
  {							//if eight measurement have been made safe it as a result
    result.d72 = sum_d72/SAMPLES;
    sum_d72 = 0;
    n_d72 = 0;
  }
}

//Interrupt service routine for d69
ISR(ADCA_CH2_vect)
{
  static uint8_t n_d69 = 0;
  static int16_t sum_d69 = 0;

  if(reset_d69 == 1){
    n_d69 = 0;
    sum_d69 = 0;
    reset_d69 = 0;
  }

  if(n_d69 & 0x01) 
  {                  		    //second (even) measurement
    sum_d69 -= (ADCA.CH2.RES);
    ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc | ADC_CH_MUXNEG_PIN3_gc;
  }
  else 
  {                         //first (odd) measurement
    sum_d69 += (ADCA.CH2.RES);
    ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN6_gc;
  }

  n_d69++;
  if (n_d69 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    result.d69 = sum_d69/SAMPLES;
    sum_d69 = 0;
    n_d69 = 0;
  }
}

//Interrupt service routine for d66
ISR(ADCB_CH0_vect)
{
  static uint8_t n_d66 = 0;
  static int16_t sum_d66 = 0;	

<<<<<<< HEAD:Not compiled/Lijnvolger/xMega_Lijnvolger.c
  if(reset_d66 == 1){
    n_d66 = 0;
    sum_d66 = 0;
    reset_d66 = 0;
  }

=======
>>>>>>> 4a943c9124fc8d2c3635e6b0425980cf676163b2:xMega_Lijnvolger.c
  if(n_d66 & 0x01) 
  {                  		//second (even) measurement
    sum_d66 -= (ADCB.CH0.RES);
    ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_d66 += (ADCB.CH0.RES);
    ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN0_gc;
  }

  n_d66++;
  if(n_d66 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    result.d66 = sum_d66/SAMPLES;
    sum_d66 = 0;
    n_d66 = 0;
  }
}

//Interrupt service routine for d63
ISR(ADCB_CH1_vect)
{
  static uint8_t n_d63 = 0;
  static int16_t sum_d63 = 0;

  if(reset_d63 == 1){
    n_d63 = 0;
    sum_d63 = 0;
    reset_d63 = 0;
  }

  if(n_d63 & 0x01) 
  {                  		    //second (even) measurement
    sum_d63 -= (ADCB.CH1.RES);
    ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_d63 += (ADCB.CH1.RES);
    ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN2_gc;
  }

  n_d63++;
  if(n_d63 == SAMPLES)
  {							//if eight measurement have been made safe it as a result
    result.d63 = sum_d63/SAMPLES;
    sum_d63 = 0;
    n_d63 = 0;
  }
}

//Interrupt service routine for d60
ISR(ADCB_CH2_vect)
{
  static uint8_t n_d60 = 0;
  static int16_t sum_d60 = 0;

  if(reset_d60 == 1){
    n_d60 = 0;
    sum_d60 = 0;
    reset_d60 = 0;
  }

  if(n_d60 & 0x01) 
  {                  		//second (even) measurement
    sum_d60 -= (ADCB.CH2.RES);
    ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc | ADC_CH_MUXNEG_PIN3_gc;
  }
  else 
  {                         //first (odd) measurement
    sum_d60 += (ADCB.CH2.RES);
    ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN5_gc;
  }

  n_d60++;
  if (n_d60 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    result.d60 = sum_d60/SAMPLES;
    sum_d60 = 0;
    n_d60 = 0;
  }
}

//Interrupt service routine for d57
ISR(ADCB_CH3_vect)
{
  static uint8_t n_d57 = 0;
  static int16_t sum_d57 = 0;

<<<<<<< HEAD:Not compiled/Lijnvolger/xMega_Lijnvolger.c
  if(reset_d57 == 1){
    n_d57 = 0;
    sum_d57 = 0;
    reset_d57 = 0;
  }
/*
  if(n_d57 & 0x01) 
=======
  /*if(n_d57 & 0x01) 
>>>>>>> 4a943c9124fc8d2c3635e6b0425980cf676163b2:xMega_Lijnvolger.c
  {                  		    //second (even) measurement
    sum_d57 -= (ADCB.CH3.RES + zero_d57);
    ADCB.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc | ADC_CH_MUXNEG_PIN3_gc;
  }
  else 
  {                         //first (odd) measurement
    sum_d57 += (ADCB.CH3.RES - zero_d57);
    ADCB.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN7_gc;
  }*/
  sum_d57 += ADCB.CH3.RES;

  n_d57++;
  if (n_d57 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    result.d57 = sum_d57/SAMPLES;
    sum_d57 = 0;
    n_d57 = 0;
  }
}

void leds_latch_data(void)
{
	PORTE.OUTSET = SHFT_LATCH_bm;
	PORTE.OUTCLR = SHFT_LATCH_bm;
}

void init_Line_Follower(void)
{
	PORTE_DIRSET = PIN7_bm;		// SIN
	PORTE_DIRCLR = PIN6_bm;		// SOUT
	PORTE_DIRSET = PIN5_bm;		// SCLK
	PORTE_DIRSET = PIN4_bm;		// LAT
	PORTE_DIRSET = PIN3_bm;		// BLANK

  leds_reset();
	
	////////////////////////////////////////

	PORTA.DIRCLR     = PIN1_bm|PIN3_bm|PIN4_bm|PIN6_bm;										      //configure PA as input for ADCA

	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN3_gc;						//PA1 to channel 0
	ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											          //differential, no gain
	ADCA.CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;													            //Interrupt on low level interrupts

	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN4_gc | ADC_CH_MUXNEG_PIN3_gc;						//PA4 to channel 1
	ADCA.CH1.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											          //differential, no gain
	ADCA.CH1.INTCTRL = ADC_CH_INTLVL_LO_gc;													            //Interrupt on low level interrupts

	ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc | ADC_CH_MUXNEG_PIN3_gc;						//PA6 to channel 2
	ADCA.CH2.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											          //differential, no gain
	ADCA.CH2.INTCTRL = ADC_CH_INTLVL_LO_gc;													            //Interrupt on low level interrupts

	ADCA.CTRLB       = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; 							

	ADCA.REFCTRL     = ADC_REFSEL_INTVCC2_gc;												            //internal vcc/2 refernce
	ADCA.PRESCALER   = ADC_PRESCALER_DIV16_gc;												          //prescaling
	ADCA.CTRLA       = ADC_ENABLE_bm;														                //enable ADC

	ADCA.EVCTRL		 = ADC_SWEEP_012_gc | ADC_EVSEL_0123_gc | ADC_EVACT_CH012_gc; //Sweep CH0,1,2; select event CH0,1,2,3; event triggers ADC CH0,1,2

	////////////////////////////////////////

	PORTB.DIRCLR     = PIN0_bm|PIN2_bm|PIN3_bm|PIN5_bm|PIN7_bm;								  //configure PB as input for ADCB

	ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN3_gc;						//PB0 to channel 0
	ADCB.CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											          //differential, no gain
	ADCB.CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;													            //Interrupt on low level interrupts

	ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc | ADC_CH_MUXNEG_PIN3_gc;						//PB1 to channel 1
	ADCB.CH1.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											          //differential, no gain
	ADCB.CH1.INTCTRL = ADC_CH_INTLVL_LO_gc;												    	        //Interrupt on low level interrupts

	ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc | ADC_CH_MUXNEG_PIN3_gc;						//PB5 to channel 2
	ADCB.CH2.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											          //differential, no gain
	ADCB.CH2.INTCTRL = ADC_CH_INTLVL_LO_gc;													            //Interrupt on low level interrupts

	ADCB.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc | ADC_CH_MUXNEG_PIN3_gc;						//PB6 to channel 3
	ADCB.CH3.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											          //differential, no gain
	ADCB.CH3.INTCTRL = ADC_CH_INTLVL_LO_gc;													            //Interrupt on low level interrupts

	ADCB.CTRLB       = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; 							  //

	ADCB.REFCTRL     = ADC_REFSEL_INTVCC2_gc;												            //internal vcc/2 refernce
	ADCB.PRESCALER   = ADC_PRESCALER_DIV16_gc;												          //prescaling
	ADCB.CTRLA       = ADC_ENABLE_bm;														                //enable ADC

	ADCB.EVCTRL		   = ADC_SWEEP_0123_gc | ADC_EVSEL_0123_gc | ADC_EVACT_CH0123_gc;			//Sweep CH0,1,2,3; select event CH0,1,2,3; event triggers ADC CH0,1,2,3

	////////////////////////////////////////

	EVSYS.CH0MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;												//event overflow timer E0 CH0
	EVSYS.CH1MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;												//event overflow timer E0 CH0
	EVSYS.CH2MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;												//event overflow timer E0 CH0
	EVSYS.CH3MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;												//event overflow timer E0 CH0
	PMIC.CTRL |= PMIC_LOLVLEN_bm;															      //turn on low level interrupts

  ////////////////////////////////////////

  leds_front(RED);
}

void leds_reset (void) {
  PORTE_OUTSET = PIN3_bm;		// BLANK high
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte

	leds_latch_data();

	PORTE_OUTCLR = PIN3_bm;		// BLANK high
}

void leds_front(uint8_t color) {
  PORTE_OUTSET = PIN3_bm;		// BLANK high

	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
  
  spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte	

	spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte
	
	spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte	

	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte

  spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte

	leds_latch_data();

	PORTE_OUTCLR = PIN3_bm;		// BLANK high
}

void init_Timer(void)
{
  reset_d75 = 1, reset_d72 = 1, 
  reset_d69 = 1, reset_d66 = 1, 
  reset_d63 = 1, reset_d60 = 1, 
  reset_d57 = 1; sample_count = 0;

  TCE0.PER      = 50 * SAMPLE_DELAY;     					              //Tper =  256 * ((SAMPLE_DELAY * 125) +1) / 32M = convererts predefined time as milliseconds
  TCE0.CTRLA    = TC_CLKSEL_DIV64_gc;                            //Prescaling 256
  TCE0.CTRLB    = TC_WGMODE_NORMAL_gc;          	                //Normal mode
  TCE0.INTCTRLA = TC_OVFINTLVL_OFF_gc;        	                  //Interrupt overflow off
}

void stop_Timer(void)
{
	TCE0.CTRLA    = TC_CLKSEL_OFF_gc;			// timer/counter off
	TCE0.INTCTRLA = TC_OVFINTLVL_OFF_gc;		// disables overflow interrupt
}

void get_Zero(void)
{
  leds_front(BLACK);

  //init_Timer();
  while (sample_count < 2) {}
  zero.d75 = control_Result(&result.d75);
  zero.d72 = control_Result(&result.d72);
  zero.d69 = control_Result(&result.d69);
  zero.d66 = control_Result(&result.d66);
  zero.d63 = control_Result(&result.d63);
  zero.d60 = control_Result(&result.d60);
  zero.d57 = control_Result(&result.d57);
  stop_Timer();
}

int16_t control_Result(volatile int16_t* result)
{
  int16_t temp_res = *result;
  if(temp_res == *result) return temp_res;
  else return 0;
}