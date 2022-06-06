#ifndef RGB_SENSOR_HPP
#define RGB_SENSOR_HPP
#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

extern "C"
{
    #include "serialF0.h"
}

#define SPI_MOSI     PIN5_bm 
#define SPI_MISO     PIN6_bm 
#define SPI_SCK      PIN7_bm 

#define max(a, b, c) ((a > b)? (a > c ? a : c) : (b > c ? b : c))
#define min(a, b, c) ((a < b)? (a < c ? a : c) : (b < c ? b : c))

#define SPI_RED         0x0249
#define SPI_GREEN       0x0492
#define SPI_BLUE        0x0924
#define SPI_OFF         0x0000

#define MAX_RED_VALUE   193
#define MAX_GREEN_VALUE 218
#define MAX_BLUE_VALUE  115

#define NUMBER_ZERO_SAMPLES 100
#define NUMBER_COLOR_SAMPLES 100
#define COLOR_AVERAGE  10

#define MEASUREMENT_TIMING 15

typedef struct RGB_Struct 
{
  uint16_t r;
  uint16_t g;
  uint16_t b;
} RGB_Struct;

typedef struct HSV_Struct 
{
  uint16_t h;
  uint16_t s;
  uint16_t v;
} HSV_Struct;

class RGB_Sensor
{
    public:
    RGB_Sensor(ADC_t *ADC_Port, SPI_t *SPI_Port, PORT_t *latch_Port, uint8_t latch_Pin, PORT_t *OE_Port, uint8_t OE_Pin, bool remap);
    
    void use_LED(bool use);
    void set_LED(RGB_Struct *color);
 
    RGB_Struct read_Sensor(void);
    HSV_Struct RGB_to_HSV(RGB_Struct *RGB);
    RGB_Struct HSV_to_RGB(HSV_Struct *HSV);

    private:
    ADC_t *ADC_Port_c; 
    SPI_t *SPI_Port_c;
    PORT_t *latch_Port_c;
    uint8_t latch_Pin_c;
    PORT_t *OE_Port_c;
    uint8_t OE_Pin_c;
    bool use_Led_c;

    RGB_Struct zero_Reading(void);
    uint16_t read_ADC(ADC_CH_t* ch);
    void sensor_LED_Color(uint16_t color);
    void spi_transfer(uint16_t data); 
};
#endif