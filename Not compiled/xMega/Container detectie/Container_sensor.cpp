#define F_CPU 32000000UL

#define COM_RGB_LEFT    'L'
#define COM_RGB_RIGHT   'R'
#define COM_ULTRA       'U'

#define COM_STANDBY     '\n'

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <math.h>

#include "RGB_sensor.hpp"
#include "Ultrasonic.hpp"

extern "C"
{
    #include "serialF0.h"
    #include "clock.h"
}

void Show_sensor_colour (RGB_Struct *RGB) 
{
  PORTF.DIRSET = PIN1_bm | PIN0_bm;
  PORTC.DIRSET = PIN0_bm;
  
  TCF0.CTRLB   = TC0_CCBEN_bm | TC0_CCAEN_bm | TC_WGMODE_SINGLESLOPE_gc;
  TCC0.CTRLB   = TC0_CCAEN_bm | TC_WGMODE_SINGLESLOPE_gc;

  TCF0.CTRLA   = TC_CLKSEL_DIV4_gc;
  TCC0.CTRLA   = TC_CLKSEL_DIV4_gc;

  TCF0.PER     = 255;
  TCC0.PER     = 255;

  TCF0.CCB     = RGB->r;
  TCF0.CCA     = RGB->g;
  TCC0.CCA     = RGB->b;
}

int main(void) 
{
  RGB_Struct RGB;
  HSV_Struct HSV;

  char command = '\n';

  init_clock();	
  init_stream(F_CPU);

  RGB_sensor RGB_left(&PORTE, &SPIE, &PORTA, &ADCA, REMAP_FALSE);
  RGB_sensor RGB_right(&PORTD, &SPID, &PORTB, &ADCB, REMAP_FALSE);

  Ultrasonic ULTRA_left(40, 0, &PORTE, PIN0_bm, PIN1_bm);
  Ultrasonic ULTRA_right(40, 0, &PORTD, PIN0_bm, PIN1_bm);
  
  float distance_left, distance_right;
  
  sei();

	while (1) 
    {
    if (CanRead_F0()) 
    {
      scanf("%c", &command);

      if ((command == COM_RGB_RIGHT) || (command == COM_RGB_LEFT)) 
      {
        if (command == COM_RGB_LEFT) RGB = RGB_left.Get_RGB_value();
        else if (command == COM_RGB_RIGHT) RGB = RGB_right.Get_RGB_value();

        HSV = RGB_to_HSV(&RGB);

        if (HSV.h <= 10) printf(" %d\n", RED);
        if ((HSV.h > 10)    &&  (HSV.h <= 25))   printf(" %d\n", ORANGE);
        if ((HSV.h > 25)   &&  (HSV.h <= 85))   printf(" %d\n", YELLOW);
        if ((HSV.h > 85)   &&  (HSV.h <= 150))  printf(" %d\n", GREEN);
        if ((HSV.h > 150)  &&  (HSV.h <= 220))  printf(" %d\n", CYAN);
        if ((HSV.h > 220)  &&  (HSV.h <= 265))  printf(" %d\n", BLUE);
        if ((HSV.h > 265)  &&  (HSV.h <= 320))  printf(" %d\n", PURPLE);
        if ((HSV.h > 320)  &&  (HSV.h <= 355))  printf(" %d\n", PINK);
        if (HSV.h > 355) printf(" %d\n", RED);

        HSV.s = 100; HSV.v = 20;
        RGB = HSV_to_RGB(&HSV);
        Show_sensor_colour(&RGB);

        command = '\n';
      }
    }
    
    distance_left = (int)ULTRA_left.Measure_distance_cm(22.307);
    distance_right = (int)ULTRA_right.Measure_distance_cm(22.307);
    printf(" %ld %ld\n", (uint32_t)(distance_left), (uint32_t)(distance_right));
    _delay_ms(1);
  }
}