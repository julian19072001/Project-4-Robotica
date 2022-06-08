#ifndef RGB_SENSOR__HPP
#define RGB_SENSOR__HPP

  #include <stdio.h>
  #include <avr/io.h>
  #include <util/delay.h>
  #include <avr/interrupt.h>
  #include <string.h>

  extern "C"
  {
      #include "serialF0.h"
  }

  #define SPI_MOSI_bm     PIN5_bm // pin7
  #define SPI_MISO_bm     PIN6_bm // pin6
  #define SPI_SCK_bm      PIN7_bm // pin5
  #define SPI_SS_bm       PIN3_bm // pin4
  #define SPI_BLANK_bm    PIN4_bm // pin3

  #define REMAP_FALSE    0
  #define REMAP_TRUE     1

  #define SAMPLES        100
  #define SAMPLE_AVR     5

  #define LED_RED     0x9249
  #define LED_GREEN   0x2492
  #define LED_BLUE    0x4924
  #define LED_WHITE   0xFFFF

  #define R   0
  #define G   1
  #define B   2

  #define WHITE     0
  #define RED       1
  #define ORANGE    2
  #define YELLOW    3
  #define GREEN     4
  #define CYAN      5
  #define BLUE      6
  #define PINK      7
  #define PURPLE    8
  

  typedef struct RGB_Struct {
    uint16_t c;
    uint16_t r;
    uint16_t g;
    uint16_t b;
  } RGB_Struct;

  typedef struct HSV_Struct {
    uint16_t h;
    uint16_t s;
    uint16_t v;
  } HSV_Struct;


  HSV_Struct RGB_to_HSV(RGB_Struct *RGB);
  RGB_Struct HSV_to_RGB (HSV_Struct *HSV);
  float max(float a, float b, float c);
  float min(float a, float b, float c);


  class RGB_sensor {
      public:
        // Constructor for the rgb_sensor
        RGB_sensor(PORT_t *spi_port,
                   SPI_t  *spi_ctrl_port, 
                   PORT_t *adc_port,
                   ADC_t  *adc_ctrl_port,
                   uint8_t remap_option);

        // Main function to retrieve rgb values
        RGB_Struct Get_RGB_value(void);

      private:
        // Stuct for asigning the correct Ports
        PORT_t *spi_port_c;       // Port used for SPI
        SPI_t  *spi_ctrl_port_c;  // SPI port used
        PORT_t *adc_port_c;       // Port used for ADC
        ADC_t  *adc_ctrl_port_c;  // ADC port used

        // Utility functions
        void init_Timer(uint8_t setting);;                                                                        // Initialize timer used to regulate the number of samples taken
        void stop_Timer(void);                                                                                    // Stop timer used to regulate the number of samples taken
        static uint8_t Map_val(uint16_t x, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max);   // Map a value to a new range
        void set_adcch_input(ADC_CH_t *ch, uint8_t pos_pin_gc, uint8_t neg_pin_gc);                               // Function for setting ADC input in differential mode
        uint8_t spi_transfer(uint8_t data);                                                                       // Transfer data to spi bus

        // LED controll functions
        void leds_reset(void);                                                                                    // Set all LED's to black
        void leds_center(uint16_t color);                                                                          // Turn on LED's
  };

#endif //RGB_SENSOR_HPP 