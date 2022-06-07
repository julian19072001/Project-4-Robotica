#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serialF0.h"
#include "clock.h"
#include "Kleur_arrays.h"

#define SAMPLES 8                 
#define SAMPLE_DELAY 1              

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

void init_Line_Follower(void);
void init_Timer(void);
void color_Line_Follower(void);
void line_Follower_Off(void);
void get_Zero(void);

int16_t control_Result(volatile int16_t* result);

int main(void) 
{
  init_Line_Follower();
	init_Timer();
	init_clock();
	init_stream(F_CPU);

	sei(); 

	while (1) 
	{	
    measurements_t final;
    final.d75 = control_Result(&result.d75) - zero.d75;
    final.d72 = control_Result(&result.d72) - zero.d72;
    final.d69 = control_Result(&result.d69) - zero.d69;
    final.d66 = control_Result(&result.d66) - zero.d66;
    final.d63 = control_Result(&result.d63) - zero.d63;
    final.d60 = control_Result(&result.d60) - zero.d60;
    final.d57 = control_Result(&result.d57) - zero.d57;
    
    printf(" %d %d %d %d %d %d %d\n", final.d75, final.d72, final.d69, final.d66, final.d63, final.d60, final.d57);
    _delay_ms(1);
	}
}

ISR(ADCA_CH0_vect)
{
  static uint8_t n_d75 = 0;
  static int16_t sum_d75 = 0;

  if(n_d75 & 0x01) 
  {                  		   
    sum_d75 -= (ADCA.CH0.RES);
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                       
    sum_d75 += (ADCA.CH0.RES);
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN1_gc;
  }

  n_d75++;
  if(n_d75 == SAMPLES) 
  {						
    result.d75 = sum_d75/SAMPLES;
    sum_d75 = 0;
    n_d75 = 0;
  }
}

ISR(ADCA_CH1_vect)
{
  static uint8_t n_d72 = 0;
  static int16_t sum_d72 = 0;

  if(n_d72 & 0x01) 
  {                  
    sum_d72 -= (ADCA.CH1.RES);
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN4_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                     
    sum_d72 += (ADCA.CH1.RES);
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN4_gc;
  }

  n_d72++;
  if(n_d72 == SAMPLES)
  {						
    result.d72 = sum_d72/SAMPLES;
    sum_d72 = 0;
    n_d72 = 0;
  }
}

ISR(ADCA_CH2_vect)
{
  static uint8_t n_d69 = 0;
  static int16_t sum_d69 = 0;

  if(n_d69 & 0x01) 
  {                  		    
    sum_d69 -= (ADCA.CH2.RES);
    ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc | ADC_CH_MUXNEG_PIN3_gc;
  }
  else 
  {                      
    sum_d69 += (ADCA.CH2.RES);
    ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN6_gc;
  }

  n_d69++;
  if (n_d69 == SAMPLES) 
  {						
    result.d69 = sum_d69/SAMPLES;
    sum_d69 = 0;
    n_d69 = 0;
  }
}

ISR(ADCB_CH0_vect)
{
  static uint8_t n_d66 = 0;
  static int16_t sum_d66 = 0;	

  if(n_d66 & 0x01) 
  {                  		
    sum_d66 -= (ADCB.CH0.RES);
    ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                
    sum_d66 += (ADCB.CH0.RES);
    ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN0_gc;
  }

  n_d66++;
  if(n_d66 == SAMPLES) 
  {				
    result.d66 = sum_d66/SAMPLES;
    sum_d66 = 0;
    n_d66 = 0;
  }
}

ISR(ADCB_CH1_vect)
{
  static uint8_t n_d63 = 0;
  static int16_t sum_d63 = 0;

  if(n_d63 & 0x01) 
  {                  		   
    sum_d63 -= (ADCB.CH1.RES);
    ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc | ADC_CH_MUXNEG_PIN3_gc;
  } 
  else 
  {                      
    sum_d63 += (ADCB.CH1.RES);
    ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN2_gc;
  }

  n_d63++;
  if(n_d63 == SAMPLES)
  {						
    result.d63 = sum_d63/SAMPLES;
    sum_d63 = 0;
    n_d63 = 0;
  }
}

ISR(ADCB_CH2_vect)
{
  static uint8_t n_d60 = 0;
  static int16_t sum_d60 = 0;

  if(n_d60 & 0x01) 
  {                  	
    sum_d60 -= (ADCB.CH2.RES);
    ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc | ADC_CH_MUXNEG_PIN3_gc;
  }
  else 
  {                        
    sum_d60 += (ADCB.CH2.RES);
    ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN5_gc;
  }

  n_d60++;
  if (n_d60 == SAMPLES) 
  {						
    result.d60 = sum_d60/SAMPLES;
    sum_d60 = 0;
    n_d60 = 0;
  }
}

ISR(ADCB_CH3_vect)
{
  static uint8_t n_d57 = 0;
  static int16_t sum_d57 = 0;

  sum_d57 += ADCB.CH3.RES;

  n_d57++;
  if (n_d57 == SAMPLES) 
  {							
    result.d57 = sum_d57/SAMPLES;
    sum_d57 = 0;
    n_d57 = 0;
  }
}

void init_Line_Follower(void)
{
	PORTE_DIRSET = PIN7_bm;		
	PORTE_DIRCLR = PIN6_bm;	
	PORTE_DIRSET = PIN5_bm;		
	PORTE_DIRSET = PIN4_bm;	
	PORTE_DIRSET = PIN3_bm;		

  line_Follower_Off();

	PORTA.DIRCLR     = PIN1_bm|PIN3_bm|PIN4_bm|PIN6_bm;										    

	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN3_gc;				
	ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											         
	ADCA.CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;													            

	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN4_gc | ADC_CH_MUXNEG_PIN3_gc;				
	ADCA.CH1.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											       
	ADCA.CH1.INTCTRL = ADC_CH_INTLVL_LO_gc;													         

	ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc | ADC_CH_MUXNEG_PIN3_gc;				
	ADCA.CH2.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											      
	ADCA.CH2.INTCTRL = ADC_CH_INTLVL_LO_gc;													         

	ADCA.CTRLB       = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; 							

	ADCA.REFCTRL     = ADC_REFSEL_INTVCC2_gc;												        
	ADCA.PRESCALER   = ADC_PRESCALER_DIV16_gc;												       
	ADCA.CTRLA       = ADC_ENABLE_bm;														               

	ADCA.EVCTRL		 = ADC_SWEEP_012_gc | ADC_EVSEL_0123_gc | ADC_EVACT_CH012_gc; 

	PORTB.DIRCLR     = PIN0_bm|PIN2_bm|PIN3_bm|PIN5_bm|PIN7_bm;							

	ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN3_gc;			
	ADCB.CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											       
	ADCB.CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;													         

	ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc | ADC_CH_MUXNEG_PIN3_gc;			
	ADCB.CH1.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											      
	ADCB.CH1.INTCTRL = ADC_CH_INTLVL_LO_gc;												    	    

	ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc | ADC_CH_MUXNEG_PIN3_gc;				
	ADCB.CH2.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											        
	ADCB.CH2.INTCTRL = ADC_CH_INTLVL_LO_gc;													         

	ADCB.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc | ADC_CH_MUXNEG_PIN3_gc;					
	ADCB.CH3.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;											         
	ADCB.CH3.INTCTRL = ADC_CH_INTLVL_LO_gc;													           

	ADCB.CTRLB       = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; 							  

	ADCB.REFCTRL     = ADC_REFSEL_INTVCC2_gc;												           
	ADCB.PRESCALER   = ADC_PRESCALER_DIV16_gc;												         
	ADCB.CTRLA       = ADC_ENABLE_bm;														               

	ADCB.EVCTRL		   = ADC_SWEEP_0123_gc | ADC_EVSEL_0123_gc | ADC_EVACT_CH0123_gc;		

	EVSYS.CH0MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;									
	EVSYS.CH1MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;										
	EVSYS.CH2MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;											
	EVSYS.CH3MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;											
	PMIC.CTRL |= PMIC_LOLVLEN_bm;															  

  _delay_ms(10);
  
  get_Zero();
  color_Line_Follower();
}

void init_Timer(void)
{
  TCE0.PER      = 125 * SAMPLE_DELAY;     					            
  TCE0.CTRLA    = TC_CLKSEL_DIV256_gc;                       
  TCE0.CTRLB    = TC_WGMODE_NORMAL_gc;          	              
  TCE0.INTCTRLA = TC_OVFINTLVL_OFF_gc;        	                
}

void get_Zero(void)
{
  line_Follower_Off();

  zero.d75 = control_Result(&result.d75);
  zero.d72 = control_Result(&result.d72);
  zero.d69 = control_Result(&result.d69);
  zero.d66 = control_Result(&result.d66);
  zero.d63 = control_Result(&result.d63);
  zero.d60 = control_Result(&result.d60);
  zero.d57 = control_Result(&result.d57);
}

void line_Follower_Off(void)
{
  PORTE_OUTCLR = PIN7_bm;	
	PORTE_OUTSET = PIN3_bm;		

	for (int i = 0; i < 96; i++) 
	{
		PORTE_OUTSET = PIN5_bm;
		PORTE_OUTCLR = PIN5_bm;
	}
	
	PORTE_OUTSET = PIN4_bm;
	PORTE_OUTCLR = PIN4_bm;
}

void color_Line_Follower(void)
{
	for (int i = 0; i < 96; i++) 
	{
		if (led_out_red[i] == 1) PORTE_OUTSET = PIN7_bm;
		else PORTE_OUTCLR = PIN7_bm;
		
		PORTE_OUTSET = PIN5_bm;
		PORTE_OUTCLR = PIN5_bm;
	}
	
	PORTE_OUTSET = PIN4_bm;
	PORTE_OUTCLR = PIN4_bm;
	
	PORTE_OUTCLR = PIN3_bm;	
}

int16_t control_Result(volatile int16_t* result)
{
  int16_t temp_res = *result;
  if(temp_res == *result) return temp_res;
  else return 0;
}