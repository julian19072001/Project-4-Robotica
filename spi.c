/*#include <avr/io.h>
#include "spi.h"

#define SPI_SS_bm      0x10	// pin4
#define SPI_MOSI_bm    0x80 // pin7
#define SPI_MISO_bm    0x40 // pin6
#define SPI_SCK_bm     0x20 // pin5


//!< Struct for connection data
typedef struct ucg_xmega_comm_struct {
	void  (*pTransfer) (uint8_t);  //!< pointer to transfer function
	SPI_t  *pSPI;          //!< pointer to SPI port (NULL in case of bitbanging)
	PORT_t *pSCK;          //!< pointer to port of SCK connection
	PORT_t *pSDI;          //!< pointer to port of SDI connection  (not used in case of bit banging)
	PORT_t *pSDA;          //!< pointer to port of SDA connection
	PORT_t *pCS;           //!< pointer to port of CS  connection
	PORT_t *pRST;          //!< pointer to port of RST (RESET) connection
	PORT_t *pCD;           //!< pointer to port of CD  (A0) connection
	PORT_t *pBLK;          //!< pointer to port of BLK (LED) connection
	uint8_t bpSCK;         //!< pin position of SCK connection
	uint8_t bpSDI;         //!< pin position of SDI connection  (not used in case of bit banging)
	uint8_t bpSDA;         //!< pin position of SDA connection
	uint8_t bpCS;          //!< pin position of CS  connection
	uint8_t bpRST;         //!< pin position of RST (RESET) connection
	uint8_t bpCD;          //!< pin position of CD  (A0) connection
	uint8_t bpBLK;         //!< pin position of BLK (LED) connection
	uint8_t bmSCK;         //!< bit mask of SCK connection
	uint8_t bmSDI;         //!< bit mask of SDI connection  (not used in case of bit banging)
	uint8_t bmSDA;         //!< bit mask of SDA connection
	uint8_t bmCS;          //!< bit mask of CS  connection
	uint8_t bmRST;         //!< bit mask of RST (RESET) connection
	uint8_t bmCD;          //!< bit mask of CD  (A0) connection
	uint8_t bmBLK;         //!< bit mask of BLK (LED) connection
	uint8_t blkDisabled;   //!< if 1 the BLK (LED) connection) is disabled
} ucg_xmega_comm_t;

static ucg_xmega_comm_t  _commInterface;  //!< local struct for connection with Xmega

void spi_init(void)
{
	PORTD.DIRSET  =  SPI_SCK_bm|SPI_MOSI_bm|SPI_SS_bm;
	PORTD.DIRCLR  =  SPI_MISO_bm;
	PORTD.OUTSET  =  SPI_SS_bm;
	//_commInterface.pSCK->DIRSET = SPI_SCK_bm;
	//_commInterface.pSDI->DIRSET = SPI_MOSI_bm;
	//SPIE.OUTSET   =  SPI_ENABLE_bm |         // enable SPI
	SPIE.CTRL     =  SPI_ENABLE_bm |         // enable SPI
					 SPI_MASTER_bm |         // master mode
	//				 SPI_CLK2X_bm  |         // no double clock speed
	//				 SPI_DORD_bm   |         // MSB first
	SPI_MODE_0_gc |         // SPI mode 0
	SPI_PRESCALER_DIV4_gc;  // prescaling 4
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
}*/


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