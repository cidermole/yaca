#include <avr/io.h>
#include "spi.h"

// Optimize code size: gcc 4.3.3 inlines, which leads to additional code usage. Squeeze some additional bytes out!!
uint8_t _spi_putc(uint8_t data) __attribute__((noinline)); // save ~80 bytes
void _spi_init() __attribute__((noinline)); // save ~22 bytes


void _spi_init() {
	/******* set data direction of SPI pins *******/
	DDR_SPI |= (1 << P_SCK) | (1 << P_MOSI); // SCK, MOSI: output
	PORT_SPI &= ~((1 << P_SCK) | (1 << P_MOSI) | (1 << P_MISO)); // init output to 0, disable pull-up (for MISO)
	DDR_CS |= (1 << P_CS); // CS: output
	PORT_CS |= (1 << P_CS); // disable CS (CS is active low)

	// set SPI clock rate output prescaler to 1/2 of main clock
	// see ATmega8 datasheet table 50 (page 130)
	SPCR = (1 << SPE) | (1 << MSTR);
	SPSR = (1 << SPI2X);
}

uint8_t _spi_putc(uint8_t data) {
	// Load outgoing byte into SPI data register
	SPDR = data;
	// Busy-wait while transmitting
	while(!(SPSR & (1 << SPIF)));

	return SPDR;
}

