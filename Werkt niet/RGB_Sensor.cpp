#include "RGB_Sensor.hpp"

RGB_Sensor::RGB_Sensor(ADC_t *ADC_Port, SPI_t *SPI_Port, PORT_t *latch_Port, uint8_t latch_Pin, PORT_t *OE_Port, uint8_t OE_Pin, bool remap)
{
    ADC_Port_c = ADC_Port; 
    SPI_Port_c = SPI_Port;
    latch_Port_c = latch_Port;
    latch_Pin_c = latch_Pin;
    OE_Port_c = OE_Port;
    OE_Pin_c = OE_Pin;
    use_Led_c = false;

    if(SPI_Port_c == &SPIE)
    {
        PORTE.DIRSET = SPI_SCK|SPI_MOSI;
        PORTE.DIRCLR = SPI_MISO;
    }
    else
    {
        PORTD.DIRSET = SPI_SCK|SPI_MOSI;
        PORTD.DIRCLR = SPI_MISO;
    }
    latch_Port_c->DIRSET = latch_Pin_c;
    OE_Port_c->DIRSET   = OE_Pin_c;
    OE_Port_c->OUTSET   = OE_Pin_c;

    SPI_Port_c->CTRL    =   SPI_ENABLE_bm |         // enable SPI
					        SPI_MASTER_bm |         // master mode
					        SPI_MODE_0_gc |         // SPI mode 0
					        SPI_PRESCALER_DIV4_gc;  // prescaling 4

    if(remap) 
    {	    			 
	    if(SPI_Port_c == &SPIE) PORTE.REMAP |= PORT_SPI_bm;
        else PORTE.REMAP |= PORT_SPI_bm;
    }

    sensor_LED_Color(SPI_OFF);

    if(ADC_Port_c == &ADCB) PORTB.DIRCLR = PIN0_bm|PIN1_bm|PIN2_bm|PIN3_bm;	
    else PORTA.DIRCLR = PIN0_bm|PIN1_bm|PIN2_bm|PIN3_bm;									    //configure PA as input for ADCA

    ADC_Port_c->CH0.MUXCTRL = ADC_CH_MUXNEG_PIN3_gc|ADC_CH_MUXPOS_PIN0_gc;
    ADC_Port_c->CH0.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;
    ADC_Port_c->CH0.INTCTRL = ADC_CH_INTLVL_LO_gc;												//Interrupt on low level interrupts

	ADC_Port_c->CH1.MUXCTRL = ADC_CH_MUXNEG_PIN2_gc|ADC_CH_MUXPOS_PIN0_gc;
    ADC_Port_c->CH1.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;
    ADC_Port_c->CH1.INTCTRL = ADC_CH_INTLVL_LO_gc;												//Interrupt on low level interrupts

	ADC_Port_c->CH2.MUXCTRL = ADC_CH_MUXNEG_PIN1_gc|ADC_CH_MUXPOS_PIN0_gc;
    ADC_Port_c->CH2.CTRL    = ADC_CH_INPUTMODE_DIFF_gc;
    ADC_Port_c->CH2.INTCTRL = ADC_CH_INTLVL_LO_gc;												//Interrupt on low level interrupts

	ADC_Port_c->CTRLB       = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; 							
	
    ADC_Port_c->REFCTRL     = ADC_REFSEL_INT1V_gc;											//internal vcc/2 refernce
	ADC_Port_c->PRESCALER   = ADC_PRESCALER_DIV16_gc;											//prescaling
	ADC_Port_c->CTRLA       = ADC_ENABLE_bm;

    PMIC.CTRL |= PMIC_LOLVLEN_bm;															      //turn on low level interrupts
    
    sei();													
};

void RGB_Sensor::use_LED(bool use)
{
    use_Led_c = use;
}

void RGB_Sensor::set_LED(RGB_Struct *color)
{
    PORTF.DIRSET = PIN1_bm | PIN0_bm;
    PORTC.DIRSET = PIN0_bm;
  
    TCF0.CTRLB   = TC0_CCBEN_bm | TC0_CCAEN_bm | TC_WGMODE_SINGLESLOPE_gc;
    TCC0.CTRLB   = TC0_CCAEN_bm | TC_WGMODE_SINGLESLOPE_gc;

    TCF0.CTRLA   = TC_CLKSEL_DIV4_gc;
    TCC0.CTRLA   = TC_CLKSEL_DIV4_gc;

    TCF0.PER     = 255;
    TCC0.PER     = 255;

    TCF0.CCB     = color->r;
    TCF0.CCA     = color->g;
    TCC0.CCA     = color->b;
}

RGB_Struct RGB_Sensor::read_Sensor(void)
{
    RGB_Struct zero1, measurement, zero2, real_Zero, corrected_RGB, real_RGB;
    
    sensor_LED_Color(SPI_OFF);

    zero1 = zero_Reading();

    int64_t sum_Diode_R = 0;
    int64_t sum_Diode_G = 0;
    int64_t sum_Diode_B = 0;
    for(int i = 0; i < NUMBER_COLOR_SAMPLES; i++)
    {
        int64_t sum_Diode_R_2 = 0;
        sensor_LED_Color(SPI_RED);
        for(int j = 0; j < COLOR_AVERAGE; j++)
        {
            _delay_us(MEASUREMENT_TIMING);
            if(j & 0x01) 
            {           
                while(!(ADC_Port_c->CH2.INTFLAGS & ADC_CH_CHIF_bm));       		   
                sum_Diode_R_2 -= read_ADC(&ADC_Port_c->CH2);
                ADC_Port_c->CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN0_gc;
            }
            else 
            {
                while(!(ADC_Port_c->CH2.INTFLAGS & ADC_CH_CHIF_bm));                       
                sum_Diode_R_2 += read_ADC(&ADC_Port_c->CH2);
                ADC_Port_c->CH2.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN3_gc;
            }
        }
        sum_Diode_R += (sum_Diode_R_2 / (float)COLOR_AVERAGE);

        int64_t sum_Diode_G_2 = 0;
        sensor_LED_Color(SPI_GREEN);
        for(int j = 0; j < COLOR_AVERAGE; j++)
        {
            _delay_us(MEASUREMENT_TIMING);
            if(j & 0x01) 
            {    
                while(!(ADC_Port_c->CH1.INTFLAGS & ADC_CH_CHIF_bm));               		   
                sum_Diode_G_2 -= read_ADC(&ADC_Port_c->CH1);
                ADC_Port_c->CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc | ADC_CH_MUXNEG_PIN0_gc;
            }
            else 
            {  
                while(!(ADC_Port_c->CH1.INTFLAGS & ADC_CH_CHIF_bm));                     
                sum_Diode_G_2 += read_ADC(&ADC_Port_c->CH1);
                ADC_Port_c->CH1.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN2_gc;
            }
        }
        sum_Diode_G += (sum_Diode_G_2 / (float)COLOR_AVERAGE);

        int64_t sum_Diode_B_2 = 0;
        sensor_LED_Color(SPI_BLUE);
        for(int j = 0; j < COLOR_AVERAGE; j++)
        {
            _delay_us(MEASUREMENT_TIMING);
            if(j & 0x01) 
            {       
                while(!(ADC_Port_c->CH0.INTFLAGS & ADC_CH_CHIF_bm));            		   
                sum_Diode_B_2 -= read_ADC(&ADC_Port_c->CH0);
                ADC_Port_c->CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN0_gc;
            }
            else 
            {   
                while(!(ADC_Port_c->CH0.INTFLAGS & ADC_CH_CHIF_bm));                    
                sum_Diode_B_2 += read_ADC(&ADC_Port_c->CH0);
                ADC_Port_c->CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN1_gc;
            }
        }
        sum_Diode_B += (sum_Diode_B_2 / (float)COLOR_AVERAGE);
    }
    measurement.r = sum_Diode_R / (float) NUMBER_COLOR_SAMPLES;
    measurement.g = sum_Diode_G / (float) NUMBER_COLOR_SAMPLES;
    measurement.b = sum_Diode_B / (float) NUMBER_COLOR_SAMPLES;

    zero2 = zero_Reading();

    real_Zero.r = (zero1.r + zero2.r) / 2.0;
    real_Zero.g = (zero1.g + zero2.g) / 2.0;
    real_Zero.b = (zero1.b + zero2.b) / 2.0;

    corrected_RGB.r = measurement.r - real_Zero.r;
    corrected_RGB.g = measurement.g - real_Zero.g;
    corrected_RGB.b = measurement.b - real_Zero.b;

    real_RGB.r = (corrected_RGB.r / (float)MAX_RED_VALUE) * 255;
    real_RGB.g = (corrected_RGB.g / (float)MAX_GREEN_VALUE) * 255;
    real_RGB.b = (corrected_RGB.b / (float)MAX_BLUE_VALUE) * 255;

    if(use_Led_c == true) set_LED(&real_RGB);

    
    clear_screen();
    printf("Diode cor:");
    printf("%5d ", real_Zero.r);
    printf("%5d ", real_Zero.g);
    printf("%5d ", real_Zero.b);
    printf("\n");
    printf("Diode Values:");
    printf("%5d ", measurement.r);
    printf("%5d ", measurement.g);
    printf("%5d ", measurement.b);
    printf("\n");
    printf("Corrected values:");
    printf("%5d ", corrected_RGB.r);
    printf("%5d ", corrected_RGB.g);
    printf("%5d ", corrected_RGB.b);
    printf("\n");
    printf("real values:");
    printf("%5d ", real_RGB.r);
    printf("%5d ", real_RGB.g);
    printf("%5d ", real_RGB.b);
    printf("\n");
    

    return real_RGB;
}

RGB_Struct RGB_Sensor::zero_Reading(void)
{   
    sensor_LED_Color(SPI_OFF);
    RGB_Struct RGB;

    int64_t sum_Diode_R = 0;
    int64_t sum_Diode_G = 0;
    int64_t sum_Diode_B = 0;
    for(int i = 0; i < NUMBER_ZERO_SAMPLES; i++)
    {
        //red diode
        if(i & 0x01) 
        {  
            while(!(ADC_Port_c->CH2.INTFLAGS & ADC_CH_CHIF_bm));               		   
            sum_Diode_R -= read_ADC(&ADC_Port_c->CH2);
            ADC_Port_c->CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc | ADC_CH_MUXNEG_PIN0_gc;
        }
        else 
        {       
            while(!(ADC_Port_c->CH2.INTFLAGS & ADC_CH_CHIF_bm));         
            sum_Diode_R += read_ADC(&ADC_Port_c->CH2);
            ADC_Port_c->CH2.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN3_gc;
        }
        //Green diode
        if(i & 0x01) 
        {         
            while(!(ADC_Port_c->CH1.INTFLAGS & ADC_CH_CHIF_bm));          		   
            sum_Diode_G -= read_ADC(&ADC_Port_c->CH1);
            ADC_Port_c->CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc | ADC_CH_MUXNEG_PIN0_gc;
        }
        else 
        {          
            while(!(ADC_Port_c->CH1.INTFLAGS & ADC_CH_CHIF_bm));             
            sum_Diode_G += read_ADC(&ADC_Port_c->CH1);
            ADC_Port_c->CH1.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN2_gc;
        }
        //Blue diode
        if(i & 0x01) 
        {         
            while(!(ADC_Port_c->CH0.INTFLAGS & ADC_CH_CHIF_bm));          		   
            sum_Diode_B -= read_ADC(&ADC_Port_c->CH0);
            ADC_Port_c->CH2.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXNEG_PIN0_gc;
        }
        else 
        {     
            while(!(ADC_Port_c->CH0.INTFLAGS & ADC_CH_CHIF_bm));                  
            sum_Diode_B += read_ADC(&ADC_Port_c->CH0);
            ADC_Port_c->CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN1_gc;
        }
        _delay_us(MEASUREMENT_TIMING);
    }
    RGB.r = sum_Diode_R / (float)NUMBER_ZERO_SAMPLES;
    RGB.g = sum_Diode_G / (float)NUMBER_ZERO_SAMPLES;
    RGB.b = sum_Diode_B / (float)NUMBER_ZERO_SAMPLES;

    return RGB;
}

uint16_t RGB_Sensor::read_ADC(ADC_CH_t* ch)
{
    int16_t res = 0;
    ch->CTRL |= ADC_CH_START_bm;
    while(!(ch->INTFLAGS & ADC_CH_CHIF_bm));
    res = ch->RES;
    ch->INTFLAGS |= ADC_CH_CHIF_bm;

    return res;
}

HSV_Struct RGB_Sensor::RGB_to_HSV(RGB_Struct *RGB) 
{
  HSV_Struct HSV;

  // R, G, B values are divided by 255
  // to change the range from 0 - 255 to 0 - 1:
  float r = RGB->r / 255.0;
  float g = RGB->g / 255.0;
  float b = RGB->b / 255.0;

  float cMax = max(r, g, b);  // maximum of r, g, b
  float cMin = min(r, g, b);  // minimum of r, g, b
  float diff = cMax-cMin;     // diff of cmax and cmin.

  if (cMax == cMin) HSV.h = 0;
  else if (cMax == r) HSV.h = fmod((60 * ((g - b) / diff) + 360), 360.0);
  else if (cMax == g) HSV.h = fmod((60 * ((b - r) / diff) + 120), 360.0);
  else if (cMax == b) HSV.h = fmod((60 * ((r - g) / diff) + 240), 360.0);

  // if cmax equal zero
  if (cMax == 0) HSV.s = 0;
  else HSV.s = (diff / cMax) * 100;
  
  // compute v
  HSV.v = cMax * 100;
  
  return HSV;
}

RGB_Struct RGB_Sensor::HSV_to_RGB(HSV_Struct *HSV)
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

  if(H >= 0 && H < 60)          r = C,g = X,b = 0;
  else if(H >= 60 && H < 120)   r = X,g = C,b = 0;
  else if(H >= 120 && H < 180)  r = 0,g = C,b = X;
  else if(H >= 180 && H < 240)  r = 0,g = X,b = C;
  else if(H >= 240 && H < 300)  r = X,g = 0,b = C;
  else                          r = C,g = 0,b = X;

  RGB.r = (r + m) * 255;
  RGB.g = (g + m) * 255;
  RGB.b = (b + m) * 255;

  return RGB;
}

void RGB_Sensor::sensor_LED_Color(uint16_t color) 
{
    OE_Port_c->OUTSET = OE_Pin_c;		// BLANK high

	//spi_transfer(color);   // transfer low byte

	latch_Port_c->OUTSET = latch_Pin_c;
	latch_Port_c->OUTCLR = latch_Pin_c;

    OE_Port_c->OUTCLR = OE_Pin_c;		// BLANK low
}                                                                                                          

void RGB_Sensor::spi_transfer(uint16_t data)
{
    uint8_t first_Half = data >> 8;
    uint8_t second_Half = data;
	SPI_Port_c->DATA = 0x02;
	while(!(SPI_Port_c->STATUS & (SPI_IF_bm)));
    printf("eerste helft");
    SPI_Port_c->DATA = 0x49;
    while(!(SPI_Port_c->STATUS & (SPI_IF_bm)));
    printf("tweede helft");
}