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

#define ORANGE ((HSV.h > 7) && (HSV.h <= 25))
#define YELLOW ((HSV.h > 7) && (HSV.h <= 25))
#define GREEN ((HSV.h > 7) && (HSV.h <= 25))
#define CYAN ((HSV.h > 7) && (HSV.h <= 25))
#define BLUE ((HSV.h > 7) && (HSV.h <= 25))
#define PURPLE ((HSV.h > 7) && (HSV.h <= 25))
#define PINK ((HSV.h > 7) && (HSV.h <= 25))


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

  RGB_sensor RGB_left(&PORTE, &SPIE, &PORTA, &ADCA, REMAP_FALSE);
  RGB_Struct RGB;
  HSV_Struct HSV;

  sei();

	while (1) {
    if (CanRead_F0()) {
      scanf("%c", &command);

      switch (command)
      {
      case 'S':
        RGB = RGB_left.Get_RGB_value();
        HSV = RGB_left.RGB_to_HSV(&RGB);

        if (HSV.s < 10) 
        {
            printf(" WHITE\n");
        } 
        else {
          if (HSV.h <= 7) printf(" RED\n");
          if (ORANGE)   printf(" ORANGE\n");
          if (YELLOW)   printf(" YELLOW\n");
          if (GREEN)  printf(" GREEN\n");
          if ((HSV.h > 150)   &&  (HSV.h <= 220))  printf(" CYAN\n");
          if ((HSV.h > 220)   &&  (HSV.h <= 275))  printf(" BLUE\n");
          if ((HSV.h > 275)   &&  (HSV.h <= 340))  printf(" PURPLE\n");
          if ((HSV.h > 340)   &&  (HSV.h <= 360))  printf(" PINK\n");
          if (HSV.h > 360) printf(" RED\n");
        }

        printf("H: %5d | S: %5d | V: %5d\n", HSV.h , HSV.s, HSV.v);
        printf("LED_Red %5d | Green %5d | Blue %5d | Cal %5d\n", RGB.r , RGB.g, RGB.b, RGB.c);

        //HSV.s = 100; HSV.v = 100;
        RGB = RGB_left.HSV_to_RGB(&HSV);
        Show_sensor_colour(&RGB);

        command = '\n';
        break;
      
      case 'R':
        // ULTRASOON
        command = '\n';
        break;

      default:
        command = '\n';
        break;
      }
    }
  }
}