#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "mcp2515.h"
#include "spi.h"

// Initialization register values for MCP2515
//
const mcp2515_init_t mcp2515_values[] PROGMEM = {

	// BRP = 0 -> TQ = 2 * (BRP + 1) / Fosc = 2 / 8MHz = 250ns
	{CNF1, 7},

	// (SyncSeg=1), PropSeg=1, PhaseSeg1=3
	{CNF2, (1 << BTLMODE) | (1 << PHSEG11)},

	// wake-up filter disabled, PhaseSeg2=3
	{CNF3, (1 << PHSEG21)},

	// enable Rx buffer interrupts
	{CANINTE, (1 << RX1IE) | (1 << RX0IE)},

	// BUKT: rollover of messages to RXB1
	{RXB0CTRL, (1 << BUKT)},

	// set device to normal mode
	{CANCTRL, CLOCK_PRESCALER_MAGIC},

	// end marker of init sequence
	{0xFF, 0}
};

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

