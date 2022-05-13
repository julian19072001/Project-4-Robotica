#define SPI_SS_bm      0x10	// pin4
#define SPI_MOSI_bm    0x80 // pin7
#define SPI_MISO_bm    0x40 // pin6
#define SPI_SCK_bm     0x20 // pin5

#define FOO            0x00

void    spi_init(void);
uint8_t spi_transfer(uint8_t data);
void    spi_write(uint8_t data);
uint8_t spi_read(void);