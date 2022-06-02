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

  RGB_Struct RGB;
  HSV_Struct HSV;

  char command = '\n';

  init_clock();	
  init_stream(F_CPU);

  RGB_sensor RGB_left(&PORTE, &SPIE, &PORTA, &ADCA, REMAP_FALSE);
  RGB_sensor RGB_right(&PORTD, &SPID, &PORTB, &ADCB, REMAP_FALSE);

  Ultrasonic ULTRA_left(40, 0, &PORTD, PIN5_bm, PIN6_bm);
  Ultrasonic ULTRA_right(40, 0, &PORTD, PIN2_bm, PIN3_bm);

  sei();

	while (1) {
    if (CanRead_F0()) {
      scanf("%c", &command);

      switch (command) {
        case COM_RGB_RIGHT || COM_RGB_LEFT:
          if (command == COM_RGB_LEFT) {
            RGB = RGB_left.Get_RGB_value();
            HSV = RGB_left.RGB_to_HSV(&RGB);
          } else if (command == COM_RGB_RIGHT) {
            RGB = RGB_right.Get_RGB_value();
            HSV = RGB_right.RGB_to_HSV(&RGB);
          };

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
          };

          HSV.s = 100; HSV.v = 100;
          
          if (command == COM_RGB_LEFT) {
            RGB = RGB_left.HSV_to_RGB(&HSV);
          } else if (command == COM_RGB_RIGHT) {
            RGB = RGB_right.HSV_to_RGB(&HSV);
          }; 
          Show_sensor_colour(&RGB);

          command = '\n';
          break;

        case COM_ULTRA:
          distance_left = (int)ULTRA_left.Measure_distance_cm(22.307);
          distance_right = (int)ULTRA_right.Measure_distance_cm(22.307);
          printf(" %ld %ld\n", (uint32_t)(distance_left), (uint32_t)(distance_right));
          command = '\n';
          break;

        default:
          command = '\n';
          break;
      };
    };
  };
};