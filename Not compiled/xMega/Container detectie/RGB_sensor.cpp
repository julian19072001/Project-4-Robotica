#include "RGB_sensor_.hpp"

volatile uint16_t reset_diode_r, reset_diode_g, reset_diode_b;
volatile int64_t tot_diode[3];
volatile uint16_t sample_count = 0;

HSV_Struct RGB_to_HSV(RGB_Struct *RGB) 
{
  HSV_Struct HSV;

  float r = RGB->r / 255.0;
  float g = RGB->g / 255.0;
  float b = RGB->b / 255.0;

  float cmax = max(r, g, b); 
  float cmin = min(r, g, b);  
  float diff = cmax-cmin;   

  if (cmax == cmin) HSV.h = 0;
  else if (cmax == r) HSV.h = fmod((60 * ((g - b) / diff) + 360), 360.0);
  else if (cmax == g) HSV.h = fmod((60 * ((b - r) / diff) + 120), 360.0);
  else if (cmax == b) HSV.h = fmod((60 * ((r - g) / diff) + 240), 360.0);

  if (cmax == 0) HSV.s = 0;
  else HSV.s = (diff / cmax) * 100;
  
  HSV.v = cmax * 100;
  
  return HSV;
} 


RGB_Struct HSV_to_RGB(HSV_Struct *HSV)
{
  RGB_Struct RGB;
  
  float H = HSV->h; 
  float S = HSV->s;
  float V = HSV->v;

  float s = S / 100;
  float v = V / 100;
  float C = s * v;
  float X = C * (1 - fabs(fmod(H / 60.0, 2) - 1));
  float m = v - C;
  float r, g, b;

  if(H >= 0 && H < 60)          r = C, g = X, b = 0;
  else if (H >= 60 && H < 120)  r = X, g = C, b = 0;
  else if (H >= 120 && H < 180) r = 0, g = C, b = X;
  else if (H >= 180 && H < 240) r = 0, g = X, b = C;
  else if (H >= 240 && H < 300) r = X, g = 0, b = C;
  else                          r = C, g = 0, b = X;

  RGB.r = (r + m) * 255;
  RGB.g = (g + m) * 255;
  RGB.b = (b + m) * 255;

  return RGB;
} 

RGB_sensor::RGB_sensor(PORT_t *spi_port, SPI_t *spi_ctrl_port, PORT_t *adc_port, ADC_t  *adc_ctrl_port, uint8_t remap_option) 
{
  spi_port_c      = spi_port;
  spi_ctrl_port_c = spi_ctrl_port;
  adc_port_c      = adc_port;
  adc_ctrl_port_c = adc_ctrl_port;

  if(spi_port_c ==  &PORTD) 
  {
    PORTB.DIRSET          =   PIN4_bm;
    PORTB.DIRCLR          =   PIN5_bm;
  } 
  else if(spi_port_c ==  &PORTE) 
  {
    PORTE.DIRSET          =   PIN3_bm;
    PORTA.DIRCLR          =   PIN4_bm;
  }

  spi_port_c->DIRSET      =   SPI_SCK_bm|SPI_MOSI_bm|SPI_BLANK_bm;
  spi_port_c->DIRCLR      =   SPI_MISO_bm;
  spi_port_c->OUTSET      =   SPI_SS_bm;
  spi_ctrl_port_c->CTRL   =   SPI_ENABLE_bm |        
					          SPI_MASTER_bm |       
					          SPI_CLK2X_bm  |       
					          SPI_MODE_0_gc |       
					          SPI_PRESCALER_DIV4_gc;  

  if (remap_option) spi_port->REMAP	 |=	 PORT_SPI_bm;

  leds_reset();

  adc_port_c->DIRCLR     = PIN1_bm|PIN2_bm|PIN3_bm;								

  if(adc_port_c == &PORTB) 
  {
    set_adcch_input(&adc_ctrl_port_c->CH0, ADC_CH_MUXPOS_PIN3_gc, ADC_CH_MUXNEG_PIN5_gc);
    adc_ctrl_port_c->CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;												
	set_adcch_input(&adc_ctrl_port_c->CH1, ADC_CH_MUXPOS_PIN2_gc, ADC_CH_MUXNEG_PIN5_gc);		
	adc_ctrl_port_c->CH1.INTCTRL = ADC_CH_INTLVL_LO_gc;												
	set_adcch_input(&adc_ctrl_port_c->CH2, ADC_CH_MUXPOS_PIN1_gc, ADC_CH_MUXNEG_PIN5_gc);			
	adc_ctrl_port_c->CH2.INTCTRL = ADC_CH_INTLVL_LO_gc;												
  } 
  else if(adc_port_c == &PORTA) 
  {
    set_adcch_input(&adc_ctrl_port_c->CH0, ADC_CH_MUXPOS_PIN3_gc, ADC_CH_MUXNEG_PIN4_gc);
    adc_ctrl_port_c->CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;											
	set_adcch_input(&adc_ctrl_port_c->CH1, ADC_CH_MUXPOS_PIN2_gc, ADC_CH_MUXNEG_PIN4_gc);			
	adc_ctrl_port_c->CH1.INTCTRL = ADC_CH_INTLVL_LO_gc;												
	set_adcch_input(&adc_ctrl_port_c->CH2, ADC_CH_MUXPOS_PIN1_gc, ADC_CH_MUXNEG_PIN4_gc);		
	adc_ctrl_port_c->CH2.INTCTRL = ADC_CH_INTLVL_LO_gc;												
  }

  adc_ctrl_port_c->CTRLB       = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; 						
	
  adc_ctrl_port_c->REFCTRL     = ADC_REFSEL_INT1V_gc;									
  adc_ctrl_port_c->PRESCALER   = ADC_PRESCALER_DIV512_gc;								
  adc_ctrl_port_c->CTRLA       = ADC_ENABLE_bm;												
	
  adc_ctrl_port_c->EVCTRL	     = ADC_SWEEP_012_gc | ADC_EVSEL_0123_gc | ADC_EVACT_CH012_gc;	

  if(adc_port_c == &PORTA) 
  {
    ADCB.CTRLA = ADC_FLUSH_bm;												
    ADCA.CTRLA = ADC_ENABLE_bm;													
  } 
  else if(adc_port_c == &PORTB) 
  {
    ADCA.CTRLA = ADC_FLUSH_bm;													
    ADCB.CTRLA = ADC_ENABLE_bm;												
  }

  EVSYS.CH0MUX	 = EVSYS_CHMUX_TCF0_OVF_gc;	
  EVSYS.CH1MUX	 = EVSYS_CHMUX_TCF0_OVF_gc;	
  EVSYS.CH2MUX	 = EVSYS_CHMUX_TCF0_OVF_gc;												
  
  PMIC.CTRL |= PMIC_LOLVLEN_bm;	

  sei();
}

RGB_Struct RGB_sensor::Get_RGB_value(void)
{
    int16_t tot[3], cor[3];
    uint16_t colour_setting[3] = { LED_RED, LED_GREEN, LED_BLUE };
    uint8_t colour_set = 0;
    
    RGB_Struct RGB;

    for (uint8_t colour = R; colour <= B; colour++) 
    {
      leds_reset();
      init_Timer(colour);
      while (sample_count < SAMPLE_AVR);
      stop_Timer();

      cor[colour] = tot_diode[colour] / SAMPLE_AVR;

      leds_center(colour_setting[colour]);
      init_Timer(colour);
      while (sample_count < SAMPLE_AVR);
      stop_Timer();

      if (tot_diode[colour] > 0) tot_diode[colour] *= -1;
      
      tot[colour] =  (tot_diode[colour] / SAMPLE_AVR) - cor[colour];

      if (tot[colour] < 0) tot[colour] *= -1;
    }

    tot[R] *= 0.6;
    tot[G] *= 0.7;
    tot[B] *= 1.0;

    tot[R] -= 9;
    tot[G] -= 26;
    tot[B] -= 24;

    leds_reset();

    RGB.r = (((float)tot[R] / max(tot[R], tot[G], tot[B])) * 255);
    RGB.g = (((float)tot[G] / max(tot[R], tot[G], tot[B])) * 255);
    RGB.b = (((float)tot[B] / max(tot[R], tot[G], tot[B])) * 255);

    return RGB;
} 

void RGB_sensor::set_adcch_input(ADC_CH_t *ch, uint8_t pos_pin_gc, uint8_t neg_pin_gc)
{
  ch->MUXCTRL = pos_pin_gc | neg_pin_gc;
  ch->CTRL    = ADC_CH_INPUTMODE_DIFFWGAIN_gc| ADC_CH_GAIN_8X_gc;	
} 

void RGB_sensor::init_Timer(uint8_t setting)
{
  reset_diode_r = 1, reset_diode_g = 1, 
  reset_diode_b = 1, sample_count = 0;

  if(adc_port_c == &PORTA) 
  {
    ADCB.CTRLA = ADC_FLUSH_bm;												
    ADCA.CTRLA = ADC_ENABLE_bm;												
  } 
  else if(adc_port_c == &PORTB) 
  {
    ADCA.CTRLA = ADC_FLUSH_bm;													
    ADCB.CTRLA = ADC_ENABLE_bm;												
  }

  switch (setting) {
  case R:
    EVSYS.CH0MUX	 = EVSYS_CHMUX_TCF0_OVF_gc;	
    EVSYS.CH1MUX	 = EVSYS_CHMUX_OFF_gc;	
    EVSYS.CH2MUX	 = EVSYS_CHMUX_OFF_gc;
    break;
  
  case G:
    EVSYS.CH0MUX	 = EVSYS_CHMUX_OFF_gc;	
    EVSYS.CH1MUX	 = EVSYS_CHMUX_TCF0_OVF_gc;	
    EVSYS.CH2MUX	 = EVSYS_CHMUX_OFF_gc;
    break;

  case B:
    EVSYS.CH0MUX	 = EVSYS_CHMUX_OFF_gc;	
    EVSYS.CH1MUX	 = EVSYS_CHMUX_OFF_gc;	
    EVSYS.CH2MUX	 = EVSYS_CHMUX_TCF0_OVF_gc;
    break;
  
  default:
    EVSYS.CH0MUX	 = EVSYS_CHMUX_TCF0_OVF_gc;	
    EVSYS.CH1MUX	 = EVSYS_CHMUX_TCF0_OVF_gc;	
    EVSYS.CH2MUX	 = EVSYS_CHMUX_TCF0_OVF_gc;
    break;
  }
  
  TCF0.PER      = 49;     					                                         

  TCF0.CTRLA    = TC_CLKSEL_DIV8_gc;           
  TCF0.CTRLB    = TC_WGMODE_NORMAL_gc;       
  TCF0.INTCTRLA = TC_OVFINTLVL_OFF_gc;      
} 

void RGB_sensor::stop_Timer(void)
{
	TCF0.CTRLA    = TC_CLKSEL_OFF_gc;			       
	TCF0.INTCTRLA = TC_OVFINTLVL_OFF_gc;		   

  EVSYS.CH0MUX	 = EVSYS_CHMUX_OFF_gc;	
  EVSYS.CH1MUX	 = EVSYS_CHMUX_OFF_gc;	
  EVSYS.CH2MUX	 = EVSYS_CHMUX_OFF_gc;
} 

void RGB_sensor::leds_reset(void) 
{
    if (spi_port_c == &PORTE) PORTE_OUTSET = SPI_BLANK_bm;	
    else if (spi_port_c == &PORTD) PORTD_OUTSET = SPI_BLANK_bm;
	
	spi_transfer(0x00);  
	spi_transfer(0x00);  

	if(spi_port_c ==  &PORTD) 
    {
        PORTB.OUTSET = PIN4_bm;
        PORTE.OUTCLR = PIN3_bm;
    } 
    else if(spi_port_c ==  &PORTE) 
    {
        PORTE.OUTSET = PIN3_bm;
        PORTE.OUTCLR = PIN3_bm;
    }

    if (spi_port_c == &PORTE)         PORTE_OUTCLR = SPI_BLANK_bm;
    else if (spi_port_c == &PORTD)    PORTD_OUTCLR = SPI_BLANK_bm;
} 

void RGB_sensor::leds_center(uint16_t color) 
{
    if (spi_port_c == &PORTE)         PORTE_OUTSET = SPI_BLANK_bm;
    else if (spi_port_c == &PORTD)  PORTD_OUTSET = SPI_BLANK_bm;		

    uint8_t bytes [sizeof(int)] = 
    {
        ((uint16_t)color >> 0) & 0xFF,
        ((uint16_t)color >> 8) & 0xFF,
    }

    spi_transfer(bytes[1]);   
    spi_transfer(bytes[0]);  

    if(spi_port_c ==  &PORTD) 
    {
        PORTB.OUTSET = PIN4_bm;
        PORTE.OUTCLR = PIN3_bm;
    } 
    else if(spi_port_c ==  &PORTE) 
    {
        PORTE.OUTSET = PIN3_bm;
        PORTE.OUTCLR = PIN3_bm;
    }

	if (spi_port_c == &PORTE)       PORTE_OUTCLR = SPI_BLANK_bm;
    else if (spi_port_c == &PORTD)  PORTD_OUTCLR = SPI_BLANK_bm;	
} 

uint8_t RGB_sensor::spi_transfer (uint8_t data)
{
	spi_ctrl_port_c->DATA = data;
	while(!(spi_ctrl_port_c->STATUS & (SPI_IF_bm)));

	return spi_ctrl_port_c->DATA;
}

ISR(ADCA_CH0_vect)
{
  static uint16_t n_diode_r = 0;
  static int16_t sum_diode_r = 0;

  if (reset_diode_r == 1) 
  {
    n_diode_r = 0;
    sum_diode_r = 0;
    tot_diode[R] = 0;
    reset_diode_r = 0;
  }

  sum_diode_r += ADCA.CH0.RES;

  n_diode_r++;
  if (n_diode_r == SAMPLES) 
  {						
    tot_diode[R] += sum_diode_r / SAMPLES;
    sum_diode_r = 0;
    n_diode_r = 0;
    sample_count++;
  }
}


ISR(ADCA_CH1_vect)
{
  static uint16_t n_diode_g = 0;
  static int16_t sum_diode_g = 0;

  if (reset_diode_g == 1) 
  {
    n_diode_g = 0;
    sum_diode_g = 0;
    tot_diode[G] = 0;
    reset_diode_g = 0;
  }

  sum_diode_g += ADCA.CH1.RES;

  n_diode_g++;
  if (n_diode_g == SAMPLES) 
  {							
    tot_diode[G] += sum_diode_g / SAMPLES;
    sum_diode_g = 0;
    n_diode_g = 0;
    sample_count++;
  }
}

ISR(ADCA_CH2_vect)
{
  static uint16_t n_diode_b = 0;
  static int16_t sum_diode_b = 0;

  if (reset_diode_b == 1) {
    n_diode_b = 0;
    sum_diode_b = 0;
    tot_diode[B] = 0;
    reset_diode_b = 0;
  }

  sum_diode_b += ADCA.CH2.RES;

  n_diode_b++;
  if (n_diode_b == SAMPLES)
  {						
    tot_diode[B] += sum_diode_b / SAMPLES;
    sum_diode_b = 0;
    n_diode_b = 0;
    sample_count++;
  }
}

ISR(ADCB_CH0_vect)
{
  static uint16_t n_diode_r = 0;
  static int16_t sum_diode_r = 0;

  if (reset_diode_r == 1) 
  {
    n_diode_r = 0;
    sum_diode_r = 0;
    tot_diode[R] = 0;
    reset_diode_r = 0;
  }

  sum_diode_r += ADCB.CH0.RES;

  n_diode_r++;
  if (n_diode_r == SAMPLES) 
  {						
    tot_diode[R] += sum_diode_r / SAMPLES;
    sum_diode_r = 0;
    n_diode_r = 0;
    sample_count++;
  }
}

ISR(ADCB_CH1_vect)
{
  static uint16_t n_diode_g = 0;
  static int16_t sum_diode_g = 0;

  if (reset_diode_g == 1) 
  {
    n_diode_g = 0;
    sum_diode_g = 0;
    tot_diode[G] = 0;
    reset_diode_g = 0;
  }

  sum_diode_g += ADCB.CH1.RES;

  n_diode_g++;
  if (n_diode_g == SAMPLES) 
  {						
    tot_diode[G] += sum_diode_g / SAMPLES;
    sum_diode_g = 0;
    n_diode_g = 0;
    sample_count++;
  }
}

ISR(ADCB_CH2_vect)
{
  static uint16_t n_diode_b = 0;
  static int16_t sum_diode_b = 0;

  if (reset_diode_b == 1) 
  {
    n_diode_b = 0;
    sum_diode_b = 0;
    tot_diode[B] = 0;
    reset_diode_b = 0;
  }

  sum_diode_b += ADCB.CH2.RES;

  n_diode_b++;
  if (n_diode_b == SAMPLES) 
  {							
    tot_diode[B] += sum_diode_b / SAMPLES;
    sum_diode_b = 0;
    n_diode_b = 0;
    sample_count++;
  }
}