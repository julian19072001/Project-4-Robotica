#ifndef __SPI_H__
#define __SPI_H__
    #include <avr/io.h>

    #define SPI_MOSI_bm    PIN7_bm // pin7
    #define SPI_MISO_bm    PIN6_bm // pin6
    #define SPI_SCK_bm     PIN5_bm // pin5
    #define SPI_SS_bm      PIN4_bm // pin4

    #define FOO            0x00

    void    spi_init(void);
    uint8_t spi_transfer(uint8_t data);
    void    spi_write(uint8_t data);
    uint8_t spi_read(void);
#endif