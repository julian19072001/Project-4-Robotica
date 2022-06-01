#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <math.h>
#include "RGB_sensor.hpp"

extern "C"
{
    #include "serialF0.h"
    #include "clock.h"
}

void Show_sensor_colour (RGB_Struct *RGB) {
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
  uint8_t rgb_poll[7];
  uint8_t largest;
  uint8_t largest_pos;

  char command = 'S';
  init_clock();	
  init_stream(F_CPU);

  RGB_sensor left(&PORTE, &SPIE, &PORTA, &ADCA, REMAP_FALSE);
  RGB_Struct RGB;
  HSV_Struct HSV;

  sei();

	while (1) {
    if (CanRead_F0()) {
      scanf("%c", &command);
    }

    while (command == 'S') {
      RGB = left.Get_RGB_value();
      HSV = left.RGB_to_HSV(&RGB);

      if (HSV.s < 10) {
        printf(" WHITE\n");
      } else {
        if (HSV.h <= 20) printf(" RED\n");
        if ((HSV.h > 20)    &&  (HSV.h <= 40))   printf(" ORANGE\n");
        if ((HSV.h > 40)    &&  (HSV.h <= 75))   printf(" YELLOW\n");
        if ((HSV.h > 75)    &&  (HSV.h <= 140))  printf(" GREEN\n");
        if ((HSV.h > 140)   &&  (HSV.h <= 225))  printf(" CYAN\n");
        if ((HSV.h > 225)   &&  (HSV.h <= 260))  printf(" BLUE\n");
        if ((HSV.h > 260)   &&  (HSV.h <= 290))  printf(" PURPLE\n");
        if ((HSV.h > 290)   &&  (HSV.h <= 330))  printf(" PINK\n");
        if (HSV.h > 330) printf(" RED\n");
      }

      printf("H: %5d | S: %5d | V: %5d\n", HSV.h , HSV.s, HSV.v);
      printf("LED_Red %5d | Green %5d | Blue %5d | Cal %5d\n", RGB.r , RGB.g, RGB.b, RGB.c);
      
      RGB = left.HSV_to_RGB(&HSV);
      Show_sensor_colour(&RGB);

      command = '\n';
    }
  }
}