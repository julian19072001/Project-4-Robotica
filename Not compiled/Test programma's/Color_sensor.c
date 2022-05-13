#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serialF0.h"
#include "clock.h"
//#include "Kleur_arrays.h"
#include "spi.h"

#define SHFT_LATCH_bm        PIN4_bm 
#define SAMPLES 8

#define COLOR_SAMPLES   10

#define RED     0x11
#define GREEN   0x22
#define BLUE    0x44
#define WHITE   0x77
#define IR      0x88

static volatile int32_t res_d75, res_d72, res_d69, res_d66, res_d63, res_d60, res_d57;
volatile int32_t tot_d75, tot_d72, tot_d69, tot_d66, tot_d63, tot_d60, tot_d57;

volatile uint8_t num;
volatile uint8_t rgb_ready_flag;

//volatile uint8_t num_d75, num_d72, num_d69, num_d66, num_d63, num_d60, num_d57;

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
  int64_t tot;
  uint16_t rgb[4];

  PORTE_DIRSET = PIN4_bm;		// LAT
	PORTE_DIRSET = PIN3_bm;		// BLANK

	init_Line_Follower();
	init_clock();
	init_stream(F_CPU);
  spi_init();
  leds_reset();
  init_Timer;
  leds_back(RED);

  sei(); 

	while (1) {
    Get_RGB_value(rgb);
    //if (rgb_ready_flag) {
      if (((rgb[1] > (rgb[2] - 10)) && (rgb[1] < (rgb[1] + 10))) && ((rgb[1] > (rgb[3] - 10)) && (rgb[1] < (rgb[3] + 10)))) {
        printf("WHITE\n");
      } else if (((rgb[1] > (rgb[2] - 20)) && (rgb[1] < (rgb[2] + 20))) && ((rgb[3] < (rgb[1] - 30)) && (rgb[3] < (rgb[2] - 30))) && (rgb[1] > rgb[2])) {
        printf("YELLOW\n");
      } else if ((rgb[1] > rgb[3]) && (rgb[3] > rgb[2])) {
        printf("PURPLE\n");
      } else if ((rgb[2] > rgb[1]) && (rgb[2] > rgb[3])) {
        printf("GREEN\n");
      } else if ((rgb[3] > rgb[1]) && (rgb[3] > rgb[2])) {
        printf("BLUE\n");
      } else if ((rgb[1] > rgb[2]) && (rgb[1] > rgb[3])) {
        printf("RED\n");
      }
    //}

    //'printf("Red %5d | Green %5d | Blue %5d | Cal %5d\n", rgb[1], rgb[2], rgb[3], rgb[0]);
  }
}

//Interrupt service routine for d75
ISR(ADCA_CH0_vect)
{
  static uint8_t n_d75 = 0;
  static int16_t sum_d75 = 0;

  if(n_d75 & 0x01) 
  {                  		//second (even) measurement
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
  }
}

//Interrupt service routine for d72
ISR(ADCA_CH1_vect)
{
  static uint8_t n_d72 = 0;
  static int16_t sum_d72 = 0;

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

  if(n_d69 & 0x01) 
  {                  		//second (even) measurement
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

  if(n_d63 & 0x01) 
  {                  		//second (even) measurement
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

  if(n_d57 & 0x01) 
  {                  		//second (even) measurement
    sum_d57 -= ADCB.CH3.RES;
    ADCB.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc | ADC_CH_MUXNEG_PIN3_gc;
  }
  else 
  {                         //first (odd) measurement
    sum_d57 += ADCB.CH3.RES;
    ADCB.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN7_gc;
  }

  n_d57++;
  if (n_d57 == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    res_d57 = sum_d57/SAMPLES;
    tot_d57 += res_d57;
    sum_d57 = 0;
    n_d57 = 0;
    num++;
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
  uint16_t cprp, rprp, gprp, bprp;

  uint16_t total;

  // Measure calibration value
  leds_reset();
  num = 0;
  init_Timer();
  while (num < COLOR_SAMPLES);
  stop_Timer();
  tot_d69 /= num; tot_d66 /= num; tot_d63 /= num;
  ctot = ((tot_d69 + tot_d66 + tot_d63) / 3);
  rgb[0] = ctot;

  // Measure value with Red light
  leds_center(RED);
  num = 0;
  init_Timer();
  while (num < COLOR_SAMPLES);
  stop_Timer();
  tot_d69 /= num; tot_d66 /= num; tot_d63 /= num;
  rtot = (((tot_d69 + tot_d66 + tot_d63) / 3) - ctot);

  // Measure value with Green light
  leds_center(GREEN);
  num = 0;
  init_Timer();
  while (num < COLOR_SAMPLES);
  stop_Timer();
  tot_d69 /= num; tot_d66 /= num; tot_d63 /= num;
  gtot = (((tot_d69 + tot_d66 + tot_d63) / 3) - ctot);

  // Measure value with Blue light
  leds_center(BLUE);
  num = 0;
  init_Timer();
  while (num < COLOR_SAMPLES);
  stop_Timer();
  tot_d69 /= num; tot_d66 /= num; tot_d63 /= num;
  btot = (((tot_d69 + tot_d66 + tot_d63) / 3) - ctot);

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
	spi_transfer(color);   // transfer low byte
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(color);   // transfer low byte
	
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