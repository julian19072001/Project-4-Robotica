#include <avr/io.h>
#include "spi.h"

#define SPI_MOSI_bm    PIN7_bm // pin7
#define SPI_MISO_bm    PIN6_bm // pin6
#define SPI_SCK_bm     PIN5_bm // pin5
#define SPI_SS_bm      PIN4_bm // pin4

void spi_init(void)
{
	PORTE.DIRSET  =  SPI_SCK_bm|SPI_MOSI_bm|SPI_SS_bm;
	PORTE.DIRCLR  =  SPI_MISO_bm;
	PORTE.OUTSET  =  SPI_SS_bm;
	SPIE.CTRL     =  SPI_ENABLE_bm |         // enable SPI
					 SPI_MASTER_bm |         // master mode
					 SPI_CLK2X_bm  |         // no double clock speed
	// 				 SPI_DORD_bm   |         // MSB first
					 SPI_MODE_0_gc |         // SPI mode 0
					 SPI_PRESCALER_DIV4_gc;  // prescaling 4
					 
	PORTE.REMAP	 |=	 PORT_SPI_bm;
}

uint8_t spi_transfer(uint8_t data)
{
	SPIE.DATA = data;
	while(!(SPIE.STATUS & (SPI_IF_bm)));

	return SPIE.DATA;
}

void spi_write(uint8_t data)
{
	PORTE.OUTCLR = SPI_SS_bm;
	spi_transfer(data);
	PORTE.OUTSET = SPI_SS_bm;
}

uint8_t spi_read(void)
{
	uint8_t data;

	PORTE.OUTCLR = SPI_SS_bm;
	data = spi_transfer(FOO);
	PORTE.OUTSET = SPI_SS_bm;

	return data;
}