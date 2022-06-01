#define F_CPU 32000000UL

#include "RGB_sensor.hpp"

// Diode variables
volatile int32_t res_diode_r, res_diode_g, res_diode_b;
volatile uint16_t reset_diode_r, reset_diode_g, reset_diode_b;
volatile int64_t tot_diode[3];
volatile uint16_t sample_count = 0;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------- Constructor ------------------------------------------------------//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RGB_sensor::RGB_sensor(PORT_t *spi_port, SPI_t *spi_ctrl_port, PORT_t *adc_port, ADC_t  *adc_ctrl_port, uint8_t remap_option) 
{
  spi_port_c      = spi_port;
  spi_ctrl_port_c = spi_ctrl_port;
  adc_port_c      = adc_port;
  adc_ctrl_port_c = adc_ctrl_port;

  spi_port_c->DIRSET      =   SPI_SCK_bm|SPI_MOSI_bm|SPI_SS_bm|SPI_BLANK_bm;
	spi_port_c->DIRCLR      =   SPI_MISO_bm;
	spi_port_c->OUTSET      =   SPI_SS_bm;
	spi_ctrl_port_c->CTRL   =   SPI_ENABLE_bm |         // enable SPI
					                    SPI_MASTER_bm |         // master mode
					                    SPI_CLK2X_bm  |         // no double clock speed
					                    SPI_MODE_0_gc |         // SPI mode 0
					                    SPI_PRESCALER_DIV4_gc;  // prescaling 4

  if (remap_option) 
  {				 
	  spi_port->REMAP	 |=	 PORT_SPI_bm;
  }

  leds_reset();

  adc_port_c->DIRCLR     = PIN0_bm|PIN1_bm|PIN2_bm|PIN3_bm;									    //configure PA as input for ADCA

	set_adcch_input(&adc_ctrl_port_c->CH0, ADC_CH_MUXPOS_PIN1_gc, ADC_CH_MUXNEG_GND_MODE3_gc);
  adc_ctrl_port_c->CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;												//Interrupt on low level interrupts

	set_adcch_input(&adc_ctrl_port_c->CH1, ADC_CH_MUXPOS_PIN2_gc, ADC_CH_MUXNEG_GND_MODE3_gc);			//PA4 to channel 1
	adc_ctrl_port_c->CH1.INTCTRL = ADC_CH_INTLVL_LO_gc;												//Interrupt on low level interrupts

	set_adcch_input(&adc_ctrl_port_c->CH2, ADC_CH_MUXPOS_PIN3_gc, ADC_CH_MUXNEG_GND_MODE3_gc);			//PA6 to channel 2
	adc_ctrl_port_c->CH2.INTCTRL = ADC_CH_INTLVL_LO_gc;												//Interrupt on low level interrupts

	adc_ctrl_port_c->CTRLB       = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; 							
	
  adc_ctrl_port_c->REFCTRL     = ADC_REFSEL_INT1V_gc;											//internal vcc/2 refernce
	adc_ctrl_port_c->PRESCALER   = ADC_PRESCALER_DIV16_gc;											//prescaling
	adc_ctrl_port_c->CTRLA       = ADC_ENABLE_bm;													//enable ADC
	
  adc_ctrl_port_c->EVCTRL	     = ADC_SWEEP_012_gc | ADC_EVSEL_0123_gc | ADC_EVACT_CH012_gc;		//Sweep CH0,1,2; select event CH0,1,2,3; event triggers ADC CH0,1,2
  
  EVSYS.CH0MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;	
  EVSYS.CH1MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;	
  EVSYS.CH2MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;												//event overflow timer E0 CH0
  
  PMIC.CTRL |= PMIC_LOLVLEN_bm;		// event overflow timer E0 CH0

  sei();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------- Public functions ----------------------------------------------------//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RGB_Struct RGB_sensor::Get_RGB_value(void)
{
    uint16_t tot[3], cor[3], prp[3];
    uint8_t colour_setting[3] = {LED_RED, LED_GREEN, LED_BLUE};
    RGB_Struct RGB;

    clear_screen();

    // Measure calibration values 
    leds_reset();
    printf("Diode cor:");
    for (uint8_t colour = R; colour <= B; colour++) 
    {
      init_Timer(colour);
      while (sample_count < COLOR_SAMPLES);
      stop_Timer();

      if (tot_diode[colour] < 0) {
        tot_diode[colour] *= -1;
      }

      tot_diode[colour] /= sample_count;
      printf("%5d ", tot_diode[colour]);
      cor[colour] = tot_diode[colour];
    }

    printf("\nDiode Values:");
    // Measure value with LED_Red light
    for (uint8_t colour = R; colour <= B; colour++) 
    {
      leds_center(colour_setting[colour]);
      init_Timer(colour);
      while (sample_count < COLOR_SAMPLES);
      stop_Timer();
    
      if (tot_diode[colour] < 0) {
        tot_diode[colour] *= -1;
      }

      tot_diode[colour] /= sample_count;
      printf("%5d ", tot_diode[colour]);

      if (tot_diode[colour] < cor[colour]) tot[colour] = 0;
      else tot[colour] = (tot_diode[colour] - cor[colour]);
    }
    printf("\n");

    leds_reset();

    //prp[R]  = (((float)tot[R] / 1700) * 255);

    // Proportional value of RGB
    for (uint8_t colour = R; colour <= B; colour++) {
      if (prp[colour] > 1700) {
        prp[colour] = 1700;
      }
      prp[colour]  = (((float)tot[colour] / 1700) * 255);
    }
    
    RGB.r = prp[R];
    RGB.g = prp[G];
    RGB.b = prp[B];

    return RGB;
} /* Get_RGB_value() */

HSV_Struct RGB_sensor::RGB_to_HSV(RGB_Struct *RGB) 
{
  HSV_Struct HSV;

  // R, G, B values are divided by 255
  // to change the range from 0 - 255 to 0 - 1:
  float r = RGB->r / 255.0;
  float g = RGB->g / 255.0;
  float b = RGB->b / 255.0;

  float cmax = max(r, g, b);  // maximum of r, g, b
  float cmin = min(r, g, b);  // minimum of r, g, b
  float diff = cmax-cmin;     // diff of cmax and cmin.

  if (cmax == cmin) HSV.h = 0;
  else if (cmax == r) HSV.h = fmod((60 * ((g - b) / diff) + 360), 360.0);
  else if (cmax == g) HSV.h = fmod((60 * ((b - r) / diff) + 120), 360.0);
  else if (cmax == b) HSV.h = fmod((60 * ((r - g) / diff) + 240), 360.0);

  if ((HSV.h < 180) && (HSV.h > 10)) {
    HSV.h = HSV.h + ((180 - HSV.h) * 0.2);
  } else if (HSV.h > 180){
    HSV.h = HSV.h - ((HSV.h - 180) * 0.4);
  }

  // if cmax equal zero
  if (cmax == 0) HSV.s = 0;
  else HSV.s = (diff / cmax) * 100;
  
  // compute v
  HSV.v = cmax * 100;
  
  return HSV;
} /* RGB_to_HSV() */


RGB_Struct RGB_sensor::HSV_to_RGB(HSV_Struct *HSV)
{
  RGB_Struct RGB;
  
  float H = HSV->h; 
  float S = HSV->s;
  float V = HSV->v;

  float s = S/100;
  float v = V/100;
  float C = s*v;
  float X = C*(1-fabs(fmod(H/60.0, 2)-1));
  float m = v-C;
  float r,g,b;

  if(H >= 0 && H < 60){
      r = C,g = X,b = 0;
  } else if(H >= 60 && H < 120){
      r = X,g = C,b = 0;
  } else if(H >= 120 && H < 180){
      r = 0,g = C,b = X;
  } else if(H >= 180 && H < 240){
      r = 0,g = X,b = C;
  } else if(H >= 240 && H < 300){
      r = X,g = 0,b = C;
  } else{
      r = C,g = 0,b = X;
  }

  RGB.r = (r+m)*255;
  RGB.g = (g+m)*255;
  RGB.b = (b+m)*255;

  return RGB;
} /*++ HSV_to_RGB */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------- Private functions ----------------------------------------------------//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RGB_sensor::set_adcch_input(ADC_CH_t *ch, uint8_t pos_pin_gc, uint8_t neg_pin_gc)
{
  ch->MUXCTRL = pos_pin_gc | neg_pin_gc;
  ch->CTRL    = ADC_CH_INPUTMODE_DIFF_gc;
} /* set_adcch_input() */

void RGB_sensor::init_Timer(uint8_t setting)
{
  reset_diode_r = 1, reset_diode_g = 1, 
  reset_diode_b = 1, sample_count = 0;

  switch (setting)
  {
  case R:
    EVSYS.CH0MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;	
    EVSYS.CH1MUX	 = EVSYS_CHMUX_OFF_gc;	
    EVSYS.CH2MUX	 = EVSYS_CHMUX_OFF_gc;
    break;
  
  case G:
    EVSYS.CH0MUX	 = EVSYS_CHMUX_OFF_gc;	
    EVSYS.CH1MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;	
    EVSYS.CH2MUX	 = EVSYS_CHMUX_OFF_gc;
    break;

  case B:
    EVSYS.CH0MUX	 = EVSYS_CHMUX_OFF_gc;	
    EVSYS.CH1MUX	 = EVSYS_CHMUX_OFF_gc;	
    EVSYS.CH2MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;
    break;
  
  default:
    EVSYS.CH0MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;	
    EVSYS.CH1MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;	
    EVSYS.CH2MUX	 = EVSYS_CHMUX_TCE0_OVF_gc;
    break;
  }
  
  TCE0.PER      = 59;     					            // Tper =  8 * (99 +1) / 32M = 35 us
                                                // ADC conversion time at 12-bit ~3.5us
                                                // ~3.5us * 3 = 10.5us

  TCE0.CTRLA    = TC_CLKSEL_DIV8_gc;            // Prescaling 8
  TCE0.CTRLB    = TC_WGMODE_NORMAL_gc;        	// Normal mode
  TCE0.INTCTRLA = TC_OVFINTLVL_OFF_gc;        	// Interrupt overflow off
} /* init_Timer() */

void RGB_sensor::stop_Timer(void)
{
	TCE0.CTRLA    = TC_CLKSEL_OFF_gc;			        // timer/counter off
	TCE0.INTCTRLA = TC_OVFINTLVL_OFF_gc;		      // disables overflow interrupt
} /* stop_Timer() */

uint8_t RGB_sensor::Map_val(uint16_t x, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max) 
{
	return (uint64_t)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
} /* Map_val() */

float RGB_sensor::max (float a, float b, float c) 
{
   return ((a > b)? (a > c ? a : c) : (b > c ? b : c));
} /* max() */

float RGB_sensor::min (float a, float b, float c) 
{
   return ((a < b)? (a < c ? a : c) : (b < c ? b : c));
} /* min() */

void RGB_sensor::leds_reset (void) 
{
  if (spi_port_c == &PORTE) {
    PORTE_OUTSET = SPI_BLANK_bm;		// BLANK high
  } else if (spi_port_c == &PORTD) {
    PORTD_OUTSET = SPI_BLANK_bm;		// BLANK high
  }
	
	spi_transfer(0x00);   // transfer low byte
	spi_transfer(0x00);   // transfer low byte

	spi_port_c->OUTSET = SPI_SS_bm;
	spi_port_c->OUTCLR = SPI_SS_bm;

  if (spi_port_c == &PORTE) {
    PORTE_OUTCLR = SPI_BLANK_bm;		// BLANK low
  } else if (spi_port_c == &PORTD) {
    PORTD_OUTCLR = SPI_BLANK_bm;		// BLANK low
  }
} /* leds_reset() */

void RGB_sensor::leds_center (uint8_t color) 
{
  if (spi_port_c == &PORTE) {
    PORTE_OUTSET = SPI_BLANK_bm;		// BLANK high
  } else if (spi_port_c == &PORTD) {
    PORTD_OUTSET = SPI_BLANK_bm;		// BLANK high
  }

	spi_transfer(color);   // transfer low byte
	spi_transfer(color);   // transfer low byte
	
	//spi_transfer(0x00);   // transfer low byte
	//spi_transfer(0x00);   // transfer low byte
	//
	//spi_transfer(0x00);   // transfer low byte
	//spi_transfer(0x00);   // transfer low byte
	//
	//spi_transfer(0x00);   // transfer low byte
	//spi_transfer(0x00);   // transfer low byte

	spi_port_c->OUTSET = SPI_SS_bm;
	spi_port_c->OUTCLR = SPI_SS_bm;

	if (spi_port_c == &PORTE) {
    PORTE_OUTCLR = SPI_BLANK_bm;		// BLANK low
  } else if (spi_port_c == &PORTD) {
    PORTD_OUTCLR = SPI_BLANK_bm;		// BLANK low
  }
} /* leds_center() */

uint8_t RGB_sensor::spi_transfer (uint8_t data)
{
	spi_ctrl_port_c->DATA = data;
	while(!(spi_ctrl_port_c->STATUS & (SPI_IF_bm)));

	return spi_ctrl_port_c->DATA;
} /* spi_transfer() */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------- Interupt functions ---------------------------------------------------//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Interrupt service routine for red diode on Port A
ISR(ADCA_CH0_vect)
{
  static uint8_t n_diode_r = 0;
  static int16_t sum_diode_r = 0;

  if(reset_diode_r == 1)
  {
    n_diode_r = 0;
    sum_diode_r = 0;
    reset_diode_r = 0;
    tot_diode[R] = 0;
  }

  if(n_diode_r & 0x01) 
  {                  		    //second (even) measurement
    sum_diode_r -= ADCA.CH0.RES;
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_GND_MODE3_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_diode_r += ADCA.CH0.RES;
    ADCA.CH0.MUXCTRL = ADC_CH_MUXNEG_GND_MODE3_gc | ADC_CH_MUXNEG_PIN1_gc;
  }

  n_diode_r++;
  if(n_diode_r == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    res_diode_r = sum_diode_r/SAMPLES;
    tot_diode[R] += res_diode_r;
    sum_diode_r = 0;
    n_diode_r = 0;
    sample_count++;
  }
}

//Interrupt service routine for green diode on Port A
ISR(ADCA_CH1_vect)
{
  static uint8_t n_diode_g = 0;
  static int16_t sum_diode_g = 0;

  if(reset_diode_g == 1)
  {
    n_diode_g = 0;
    sum_diode_g = 0;
    reset_diode_g = 0;
    tot_diode[G] = 0;
  }

  if(n_diode_g & 0x01) 
  {                  		//second (even) measurement
    sum_diode_g -= ADCA.CH1.RES;
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc | ADC_CH_MUXNEG_GND_MODE3_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_diode_g += ADCA.CH1.RES;
    ADCA.CH1.MUXCTRL = ADC_CH_MUXNEG_GND_MODE3_gc | ADC_CH_MUXNEG_PIN2_gc;
  }

  n_diode_g++;
  if(n_diode_g == SAMPLES)
  {							//if eight measurement have been made safe it as a result
    res_diode_g = sum_diode_g/SAMPLES;
    tot_diode[G] += res_diode_g;
    sum_diode_g = 0;
    n_diode_g = 0;
    sample_count++;
  }
}

//Interrupt service routine for blue diode on Port A
ISR(ADCA_CH2_vect)
{
  static uint8_t n_diode_b = 0;
  static int16_t sum_diode_b = 0;

  if(reset_diode_b == 1)
  {
    n_diode_b = 0;
    sum_diode_b = 0;
    reset_diode_b = 0;
    tot_diode[B] = 0;
  }

  if(n_diode_b & 0x01) 
  {                  		    //second (even) measurement
    sum_diode_b -= ADCA.CH2.RES;
    ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_GND_MODE3_gc;
  }
  else 
  {                         //first (odd) measurement
    sum_diode_b += ADCA.CH2.RES;
    ADCA.CH2.MUXCTRL = ADC_CH_MUXNEG_GND_MODE3_gc | ADC_CH_MUXNEG_PIN3_gc;
  }

  n_diode_b++;
  if (n_diode_b == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    res_diode_b = sum_diode_b/SAMPLES;
    tot_diode[B] += res_diode_b;
    sum_diode_b = 0;
    n_diode_b = 0;
    sample_count++;
  }
}

//Interrupt service routine for red diode on Port B
ISR(ADCB_CH0_vect)
{
  static uint8_t n_diode_r = 0;
  static int16_t sum_diode_r = 0;

  if(reset_diode_r == 1)
  {
    n_diode_r = 0;
    sum_diode_r = 0;
    reset_diode_r = 0;
    tot_diode[R] = 0;
  }

  if(n_diode_r & 0x01) 
  {                  		    //second (even) measurement
    sum_diode_r -= ADCB.CH0.RES;
    ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN0_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_diode_r += ADCB.CH0.RES;
    ADCB.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN1_gc;
  }

  n_diode_r++;
  if(n_diode_r == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    res_diode_r = sum_diode_r/SAMPLES;
    tot_diode[R] += res_diode_r;
    sum_diode_r = 0;
    n_diode_r = 0;
    sample_count++;
  }
}

//Interrupt service routine for green diode on Port B
ISR(ADCB_CH1_vect)
{
  static uint8_t n_diode_g = 0;
  static int16_t sum_diode_g = 0;

  if(reset_diode_g == 1)
  {
    n_diode_g = 0;
    sum_diode_g = 0;
    reset_diode_g = 0;
    tot_diode[G] = 0;
  }

  if(n_diode_g & 0x01) 
  {                  		//second (even) measurement
    sum_diode_g -= ADCB.CH1.RES;
    ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc | ADC_CH_MUXNEG_PIN0_gc;
  } 
  else 
  {                         //first (odd) measurement
    sum_diode_g += ADCB.CH1.RES;
    ADCB.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN2_gc;
  }

  n_diode_g++;
  if(n_diode_g == SAMPLES)
  {							//if eight measurement have been made safe it as a result
    res_diode_g = sum_diode_g/SAMPLES;
    tot_diode[G] += res_diode_g;
    sum_diode_g = 0;
    n_diode_g = 0;
    sample_count++;
  }
}

//Interrupt service routine for blue diode on Port A
ISR(ADCB_CH2_vect)
{
  static uint8_t n_diode_b = 0;
  static int16_t sum_diode_b = 0;

  if(reset_diode_b == 1)
  {
    n_diode_b = 0;
    sum_diode_b = 0;
    reset_diode_b = 0;
    tot_diode[B] = 0;
  }

  if(n_diode_b & 0x01) 
  {                  		    //second (even) measurement
    sum_diode_b -= ADCB.CH2.RES;
    ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN0_gc;
  }
  else 
  {                         //first (odd) measurement
    sum_diode_b += ADCB.CH2.RES;
    ADCB.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN3_gc;
  }

  n_diode_b++;
  if (n_diode_b == SAMPLES) 
  {							//if eight measurement have been made safe it as a result
    res_diode_b = sum_diode_b/SAMPLES;
    tot_diode[B] += res_diode_b;
    sum_diode_b = 0;
    n_diode_b = 0;
    sample_count++;
  }
}