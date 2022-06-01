#ifndef RGB_SENSOR_HPP
#define RGB_SENSOR_HPP

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

#define SAMPLES        10
#define COLOR_SAMPLES  50
#define COLOR_AVERAGE  1

// Defines for led control
#define LED_RED     0x11
#define LED_GREEN   0x22
#define LED_BLUE    0x44
#define LED_WHITE   0x77
#define LED_IR      0x88

#define R           0
#define G           1
#define B           2

#define INCONCLUSIVE  0
#define WHITE         1
#define YELLOW        2
#define PURPLE        3
#define GREEN         4
#define BLUE          5
#define RED           6

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

class RGB_sensor
{
    public:
    // Constructor for the rgb_sensor
    RGB_sensor(PORT_t *spi_port,
               SPI_t  *spi_ctrl_port, 
               PORT_t *adc_port,
               ADC_t  *adc_ctrl_port,
               uint8_t remap_option);

    // Main function to retrieve rgb values
    RGB_Struct Get_RGB_value(void);
    HSV_Struct RGB_to_HSV(RGB_Struct *RGB);
    RGB_Struct HSV_to_RGB (HSV_Struct *HSV);

    private:
    // Stuct for asigning the correct Ports
    PORT_t *spi_port_c;       // Port used for SPI
    SPI_t  *spi_ctrl_port_c;  // SPI port used
    PORT_t *adc_port_c;       // Port used for ADC
    ADC_t  *adc_ctrl_port_c;  // ADC port used

    // Utility functions
    void init_Timer(uint8_t setting);;                                                                                    // Initialize timer used to regulate the number of samples taken
    void stop_Timer(void);                                                                                    // Stop timer used to regulate the number of samples taken
    static uint8_t Map_val(uint16_t x, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max);   // Map a value to a new range
    float max(float a, float b, float c);
    float min(float a, float b, float c);
    void set_adcch_input(ADC_CH_t *ch, uint8_t pos_pin_gc, uint8_t neg_pin_gc);                               // Function for setting ADC input in differential mode
    uint8_t spi_transfer(uint8_t data);                                                                       // Transfer data to spi bus

    // LED controll functions
    void leds_reset(void);                                                                                    // Set all LED's to black
    void leds_center(uint8_t color);                                                                          // Turn on LED's
};

#endif //RGB_SENSOR_HPP 