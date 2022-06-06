#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <math.h>
#include "RGB_Sensor.hpp"

extern "C"
{
    #include "serialF0.h"
    #include "clock.h"
}

int main(void) 
{
  uint8_t rgb_poll[7];
  uint8_t largest;
  uint8_t largest_pos;

  char command = 'S';
  init_clock();	
  init_stream(F_CPU);

  RGB_Sensor RGB_left(&ADCA, &SPIE, &PORTE, PIN3_bm, &PORTE, PIN4_bm, false);
  RGB_Struct RGB;
  HSV_Struct HSV;

  sei();

	while (1) {
    if (CanRead_F0()) {
      scanf("%c", &command);

      switch (command)
      {
      case 'S':
      printf("test");
        RGB = RGB_left.read_Sensor();
        HSV = RGB_left.RGB_to_HSV(&RGB);

        /*if (HSV.s < 10) {
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
        }*/

        if(HSV.s < 10) 
        {
          printf(" WHITE\n");
        } else {
          if (HSV.h <= 7) printf(" RED\n");
          if ((HSV.h > 7)    &&  (HSV.h <= 25))   printf(" ORANGE\n");
          if ((HSV.h > 25)    &&  (HSV.h <= 70))   printf(" YELLOW\n");
          if ((HSV.h > 70)    &&  (HSV.h <= 150))  printf(" GREEN\n");
          if ((HSV.h > 150)   &&  (HSV.h <= 220))  printf(" CYAN\n");
          if ((HSV.h > 220)   &&  (HSV.h <= 275))  printf(" BLUE\n");
          if ((HSV.h > 275)   &&  (HSV.h <= 340))  printf(" PURPLE\n");
          if ((HSV.h > 340)   &&  (HSV.h <= 360))  printf(" PINK\n");
          if (HSV.h > 360) printf(" RED\n");
        }

        printf("H: %5d | S: %5d | V: %5d\n", HSV.h , HSV.s, HSV.v);
        printf("LED_Red %5d | Green %5d | Blue %5d | Cal %5d\n", RGB.r , RGB.g, RGB.b);

        HSV.s = 100; HSV.v = 100;
        RGB = RGB_left.HSV_to_RGB(&HSV);

        command = '\n';
        break;

        default:
        command = '\n';
        break;
      }
    }
  }
}