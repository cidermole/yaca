#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "mcp2515.h"

// Initialization register values for MCP2515
//
mcp2515_init_t mcp2515_values[] PROGMEM = {

	// BRP = 0 -> TQ = 2 * (BRP + 1) / Fosc = 2 / 8MHz = 250ns
	{CNF1, 7},

	// (SyncSeg=1), PropSeg=1, PhaseSeg1=3
	{CNF2, (1 << BTLMODE) | (1 << PHSEG11)},

	// wake-up filter disabled, PhaseSeg2=3
	{CNF3, (1 << PHSEG21)},

	// enable Rx buffer interrupts
	{CANINTE, (1 << RX1IE) | (1 << RX0IE)},

	// only receive extended frames, BUKT: rollover of messages to RXB1
	{RXB0CTRL, (1 << RXM1) | (1 << BUKT)},

	// only receive extended frames
	{RXB1CTRL, (1<<RXM1)},

	// deactivate RXnBF pins (set to high-Z), unused
	{BFPCTRL, 0},

	// TXnRTS as inputs (unused)
	{TXRTSCTRL, 0},

	// end marker of init sequence
	{0xFF, 0}
};

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

void mcp2515_wrreg(uint8_t address, uint8_t data) {
	_spi_start();

	_spi_putc(MCP2515_CMD_WRITE);
	_spi_putc(address);
	_spi_putc(data);

	_spi_stop();
}

uint8_t mcp2515_rdstatus(uint8_t cmd) {
	uint8_t c;

	_spi_start();
	_spi_putc(cmd);
	c = _spi_getc();
	_spi_stop();

	return c;
}

void mcp2515_init() {
	_spi_init();

	// Software reset MCP2515 -> sets MCP2515 into Configuration Mode (the only way CNFx regs can be written)
	_spi_start();
	_spi_putc(MCP2515_CMD_RESET);
	_spi_stop();

	mcp2515_init_convert_t d;
	mcp2515_init_t* p = mcp2515_values;

	_delay_us(100); // wait for MCP2515 to get ready (undocumented, but needed)

	// Read init values one by one and execute them
	d.asint = (uint16_t) pgm_read_word(p++);
	while(d.asinit.reg != 0xFF) {
		mcp2515_wrreg(d.asinit.reg, d.asinit.val);
		d.asint = (uint16_t) pgm_read_word(p++);
	}

	mcp2515_wrreg(CANCTRL, CLOCK_PRESCALER_MAGIC); // set device to normal mode
//	mcp2515_wrreg(CANCTRL, 0x40 | CLOCK_PRESCALER_MAGIC); // set device to loopback mode

#ifdef YACA_INT1
	MCUCR &= ~((1 << ISC11) | (1 << ISC10)); // Set INT1 to active low
#else
	MCUCR &= ~((1 << ISC01) | (1 << ISC00)); // Set INT0 to active low
#endif
	mcp2515_set_int(1); // enable INT0 (note: need yet to set global I-flag)
}

void mcp2515_set_int(uint8_t val) {
#ifdef YACA_INT1
	if(val)
		GICR |= (1 << INT1);
	else
		GICR &= ~(1 << INT1);
#else
	if(val)
		GICR |= (1 << INT0);
	else
		GICR &= ~(1 << INT0);
#endif
}

