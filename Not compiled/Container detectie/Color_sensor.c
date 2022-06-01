#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "serialF0.h"
#include "clock.h"
#include "spi.h"

#define SHFT_LATCH_bm   PIN4_bm 
#define SAMPLES 8

#define COLOR_SAMPLES   4

#define COLOR_AVERAGE   6

// Defines for led control
#define RED     0x11
#define GREEN   0x22
#define BLUE    0x44
#define WHITE   0x77
#define IR      0x88

static volatile int32_t res_d75, res_d72, res_d69, res_d66, res_d63, res_d60, res_d57;
volatile uint16_t reset_d75, reset_d72, reset_d69, reset_d66, reset_d63, reset_d60, reset_d57;
volatile int32_t tot_d75, tot_d72, tot_d69, tot_d66, tot_d63, tot_d60, tot_d57;

volatile uint8_t sample_count;
volatile uint8_t rgb_ready_flag;

void init_Line_Follower(void);
void init_Timer(void);
void init_Timer2(void);
void stop_Timer(void);
void leds_reset(void);
void leds_center(uint8_t color);
void leds_front(uint8_t color);
void leds_back(uint8_t color);
void leds_latch_data(void);
static uint8_t Map_val(uint16_t x, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max);
void Get_RGB_value(uint16_t *rgb);
//void color_Line_Follower(void);

int main(void) 
{
  uint16_t rgb[4];
  uint8_t rgb_poll[7];
  uint8_t largest;
  uint8_t largest_pos;
  char command = 'S';

	init_clock();
  init_Timer();	
  init_stream(F_CPU);

  PORTE_DIRSET = PIN4_bm;		// LAT
	PORTE_DIRSET = PIN3_bm;		// BLANK

  spi_init();
  init_Line_Follower();
  leds_reset();
  
  sei();

	while (1) {
    if (CanRead_F0()) {
      scanf("%c", &command);
    }

    while (command == 'S') {
      for (uint8_t i = 0; i < COLOR_AVERAGE; i++) {
        Get_RGB_value(rgb);
        if (((rgb[1] > (rgb[2] - 10)) && (rgb[1] < (rgb[2] + 60))) && ((rgb[1] > (rgb[3] - 10)) && (rgb[1] < (rgb[3] + 60)))) {
          rgb_poll[1]++;
        } else if (((rgb[1] > (rgb[2] - 20)) && (rgb[1] < (rgb[2] + 60))) && ((rgb[3] < (rgb[1] - 100)) && (rgb[3] < (rgb[2] - 50))) && (rgb[1] > rgb[2]) && (rgb[1] > rgb[3])) {
          rgb_poll[2]++;
        } else if ((rgb[1] > rgb[3]) && (rgb[3] > rgb[2])) {
          rgb_poll[3]++;
        } else if ((rgb[2] > rgb[1]) && (rgb[2] > rgb[3])) {
          rgb_poll[4]++;
        } else if ((rgb[3] > rgb[1]) && (rgb[3] > rgb[2])) {
          rgb_poll[5]++;
        } else if ((rgb[1] > rgb[2]) && (rgb[1] > rgb[3])) {
          rgb_poll[6]++;
        } else {
          rgb_poll[0]++;
        }
      }

      largest = rgb_poll[0];
      for(uint8_t i = 1; i < 7; i++) {
           if(largest < rgb_poll[i]) {
          largest = rgb_poll[i];
          largest_pos = i;
        }   
      }

      switch (largest_pos) {
      case 0:
        printf("INCONCLUSIVE");
        break;
      
      case 1:
        printf("WHITE");
        break;
      
      case 2:
        printf("YELLOW");
        break;
      
      case 3:
        printf("PURPLE");
        break;
      
      case 4:
        printf("GREEN");
        break;
      
      case 5:
        printf("BLUE");
        break;
      
      case 6:
        printf("RED");
        break;
      
      default:
        break;
      }

      // printf("\nRed %5d | Green %5d | Blue %5d | Cal %5d\n\n", rgb[1], rgb[2], rgb[3], rgb[0]);

      leds_reset();
      memset(rgb_poll, 0, sizeof(rgb_poll));
      command = '\n';
    }

    //'printf("Red %5d | Green %5d | Blue %5d | Cal %5d\n", rgb[1], rgb[2], rgb[3], rgb[0]);
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
    sum_d75 -= ADCA.CH0.RES;
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_d75 += ADCA.CH0.RES;
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN1_gc;
  }

  n_d75++;
  if(n_d75 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    res_d75 = sum_d75/SAMPLES;
    tot_d75 += res_d75;
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
    sum_d72 -= ADCA.CH1.RES;
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN4_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_d72 += ADCA.CH1.RES;
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN4_gc;
  }

  n_d72++;
  if(n_d72 == SAMPLES)
  {							//if eight measurement have been made safe it as a result
    res_d72 = sum_d72/SAMPLES;
    tot_d72 += res_d72;
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
    sum_d69 -= ADCA.CH2.RES;
    ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc | ADC_CH_MUXNEG_PIN3_gc;
  }
  else 
  {                         //first (odd) measurement
    sum_d69 += ADCA.CH2.RES;
    ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN6_gc;
  }

  n_d69++;
  if (n_d69 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    res_d69 = sum_d69/SAMPLES;
    tot_d69 += res_d69;
    sum_d69 = 0;
    n_d69 = 0;
  }
}

//Interrupt service routine for d66
ISR(ADCB_CH0_vect)
{
  static uint8_t n_d66 = 0;
  static int16_t sum_d66 = 0;	

  if(reset_d66 == 1){
    n_d66 = 0;
    sum_d66 = 0;
    reset_d66 = 0;
  }

  if(n_d66 & 0x01) 
  {                  		//second (even) measurement
    sum_d66 -= ADCB.CH0.RES;
    ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_d66 += ADCB.CH0.RES;
    ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN0_gc;
  }

  n_d66++;
  if(n_d66 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    res_d66 = sum_d66/SAMPLES;
    tot_d66 += res_d66;
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
    sum_d63 -= ADCB.CH1.RES;
    ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_d63 += ADCB.CH1.RES;
    ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN2_gc;
  }

  n_d63++;
  if(n_d63 == SAMPLES)
  {							//if eight measurement have been made safe it as a result
    res_d63 = sum_d63/SAMPLES;
    tot_d63 += res_d63;
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
    sum_d60 -= ADCB.CH2.RES;
    ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc | ADC_CH_MUXNEG_PIN3_gc;
  }
  else 
  {                         //first (odd) measurement
    sum_d60 += ADCB.CH2.RES;
    ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN5_gc;
  }

  n_d60++;
  if (n_d60 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    res_d60 = sum_d60/SAMPLES;
    tot_d60 += res_d60;
    sum_d60 = 0;
    n_d60 = 0;
  }
}

//Interrupt service routine for d57
ISR(ADCB_CH3_vect)
{
  static uint8_t n_d57 = 0;
  static int16_t sum_d57 = 0;

  if(reset_d57 == 1){
    n_d57 = 0;
    sum_d57 = 0;
    reset_d57 = 0;
  }
/*
  if(n_d57 & 0x01) 
  {                  		    //second (even) measurement
    sum_d57 -= ADCB.CH3.RES;
    ADCB.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc | ADC_CH_MUXNEG_PIN3_gc;
  }
  else 
  {                         //first (odd) measurement
    sum_d57 += ADCB.CH3.RES;
    ADCB.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN7_gc;
  }*/
  sum_d57 += ADCB.CH3.RES;

  n_d57++;
  if (n_d57 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    res_d57 = sum_d57/SAMPLES;
    tot_d57 += res_d57;
    sum_d57 = 0;
    n_d57 = 0;
  }
}

ISR(TCD1_OVF_vect) {

}

void init_Line_Follower(void)
{	
	////////////////////////////////////////

	PORTA.DIRCLR     = PIN1_bm|PIN3_bm|PIN4_bm|PIN6_bm;										//configure PA as input for ADCA

	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN3_gc;						//PA1 to channel 0
	ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											//differential, no gain
	ADCA.CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;													//Interrupt on low level interrupts

	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN4_gc | ADC_CH_MUXNEG_PIN3_gc;						//PA4 to channel 1
	ADCA.CH1.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											//differential, no gain
	ADCA.CH1.INTCTRL = ADC_CH_INTLVL_LO_gc;													//Interrupt on low level interrupts

	ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc | ADC_CH_MUXNEG_PIN3_gc;						//PA6 to channel 2
	ADCA.CH2.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											//differential, no gain
	ADCA.CH2.INTCTRL = ADC_CH_INTLVL_LO_gc;													//Interrupt on low level interrupts

	ADCA.CTRLB       = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; 							//

	ADCA.REFCTRL     = ADC_REFSEL_INTVCC2_gc;												//internal vcc/2 refernce
	ADCA.PRESCALER   = ADC_PRESCALER_DIV16_gc;												//prescaling
	ADCA.CTRLA       = ADC_ENABLE_bm;														//enable ADC

	ADCA.EVCTRL		 = ADC_SWEEP_012_gc | ADC_EVSEL_0123_gc | ADC_EVACT_CH012_gc;			//Sweep CH0,1,2; select event CH0,1,2,3; event triggers ADC CH0,1,2

	////////////////////////////////////////

	PORTB.DIRCLR     = PIN0_bm|PIN2_bm|PIN3_bm|PIN5_bm|PIN7_bm;								//configure PB as input for ADCB

	ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN3_gc;						//PB0 to channel 0
	ADCB.CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											//differential, no gain
	ADCB.CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;													//Interrupt on low level interrupts

	ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc | ADC_CH_MUXNEG_PIN3_gc;						//PB1 to channel 1
	ADCB.CH1.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											//differential, no gain
	ADCB.CH1.INTCTRL = ADC_CH_INTLVL_LO_gc;													//Interrupt on low level interrupts

	ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc | ADC_CH_MUXNEG_PIN3_gc;						//PB5 to channel 2
	ADCB.CH2.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											//differential, no gain
	ADCB.CH2.INTCTRL = ADC_CH_INTLVL_LO_gc;													//Interrupt on low level interrupts

	ADCB.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc | ADC_CH_MUXNEG_PIN3_gc;						//PB6 to channel 3
	ADCB.CH3.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											//differential, no gain
	ADCB.CH3.INTCTRL = ADC_CH_INTLVL_LO_gc;													//Interrupt on low level interrupts

	ADCB.CTRLB       = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; 							//

	ADCB.REFCTRL     = ADC_REFSEL_INTVCC2_gc;												//internal vcc/2 refernce
	ADCB.PRESCALER   = ADC_PRESCALER_DIV16_gc;												//prescaling
	ADCB.CTRLA       = ADC_ENABLE_bm;														//enable ADC

	ADCB.EVCTRL		 = ADC_SWEEP_0123_gc | ADC_EVSEL_0123_gc | ADC_EVACT_CH0123_gc;			//Sweep CH0,1,2,3; select event CH0,1,2,3; event triggers ADC CH0,1,2,3

	////////////////////////////////////////

	//color_Line_Follower();																	//set the line follower color

	EVSYS.CH0MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;												//event overflow timer E0 CH0
	EVSYS.CH1MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;												//event overflow timer E0 CH0
	EVSYS.CH2MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;												//event overflow timer E0 CH0
	EVSYS.CH3MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;												//event overflow timer E0 CH0
	PMIC.CTRL |= PMIC_LOLVLEN_bm;															//turn on low level interrupts
}

void init_Timer(void)
{
  reset_d75 = 1, reset_d72 = 1, 
  reset_d69 = 1, reset_d66 = 1, 
  reset_d63 = 1, reset_d60 = 1, 
  reset_d57 = 1; sample_count = 0;
  
  TCE0.PER      = 999;     					            // Tper =  8 * (3124 +1) / 32M = 0.025 s
  TCE0.CTRLA    = TC_CLKSEL_DIV8_gc;          // Prescaling 8
  TCE0.CTRLB    = TC_WGMODE_NORMAL_gc;        	// Normal mode
  TCE0.INTCTRLA = TC_OVFINTLVL_OFF_gc;        	// Interrupt overflow off
}

void init_Timer2(void)
{
  TCD1.PER      = 3124;     					            // Tper =  8 * (3124 +1) / 32M = 0.025 s
  TCD1.CTRLA    = TC_CLKSEL_DIV8_gc;          // Prescaling 8
  TCD1.CTRLB    = TC_WGMODE_NORMAL_gc;        	// Normal mode
  TCD1.INTCTRLA = TC_OVFINTLVL_OFF_gc;        	// Interrupt overflow off
}

void stop_Timer(void)
{
	TCD1.CTRLA    = TC_CLKSEL_OFF_gc;			// timer/counter off
	TCD1.INTCTRLA = TC_OVFINTLVL_OFF_gc;		// disables overflow interrupt
}

static uint8_t Map_val(uint16_t x, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max) {
	return (uint64_t)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

void Get_RGB_value(uint16_t *rgb)
{
  uint16_t ctot, rtot, gtot, btot;
  uint16_t rprp, gprp, bprp;

  uint16_t total;

  // Measure calibration value
  leds_reset();
  sample_count = 0;
  init_Timer();
  while (sample_count < COLOR_SAMPLES);
  stop_Timer();
  tot_d63 /= sample_count;
  ctot = tot_d63;
  rgb[0] = ctot;

  // Measure value with Red light
  leds_center(RED);
  sample_count = 0;
  init_Timer();
  while (sample_count < COLOR_SAMPLES);
  stop_Timer();
  tot_d63 /= sample_count;
  rtot = (tot_d63 - ctot);

  // Measure value with Green light
  leds_center(GREEN);
  sample_count = 0;
  init_Timer();
  while (sample_count < COLOR_SAMPLES);
  stop_Timer();
  tot_d63 /= sample_count;
  gtot = (tot_d63 - ctot);

  // Measure value with Blue light
  leds_center(BLUE);
  sample_count = 0;
  init_Timer();
  while (sample_count < COLOR_SAMPLES);
  stop_Timer();
  tot_d63 /= sample_count;
  btot = (tot_d63 - ctot);

  // Proportional value of RGB
  total = rtot + gtot + btot; 
  rprp = (((float)rtot / total) * 100);
  rgb[1] = Map_val(rprp, 0, 100, 0, 255);
  gprp = (((float)gtot / total) * 100);
  rgb[2] = Map_val(gprp, 0, 100, 0, 255);
  bprp = (((float)btot / total) * 100);
  rgb[3] = Map_val(bprp, 0, 100, 0, 255);
}

inline void leds_latch_data(void)
{
	PORTE.OUTSET = SHFT_LATCH_bm;
	PORTE.OUTCLR = SHFT_LATCH_bm;
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

void leds_center (uint8_t color) {
  PORTE_OUTSET = PIN3_bm;		// BLANK high

	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(color);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(color);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte

	leds_latch_data();

	PORTE_OUTCLR = PIN3_bm;		// BLANK high
}

void leds_front (uint8_t color) {
  PORTE_OUTSET = PIN3_bm;		// BLANK high

	spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte
	
	spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte
	
	spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte
	
	spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte
	
	spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte
	
	spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte

	leds_latch_data();

	PORTE_OUTCLR = PIN3_bm;		// BLANK high
}

void leds_back (uint8_t color) {
  PORTE_OUTSET = PIN3_bm;		// BLANK high

	spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte
  
  spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte	

	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte	

	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte

  spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte

	leds_latch_data();

	PORTE_OUTCLR = PIN3_bm;		// BLANK high
}