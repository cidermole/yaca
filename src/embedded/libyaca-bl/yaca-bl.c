#include "yaca-bl.h"
#include "mcp2515.h"
#include <avr/io.h>
#include <avr/interrupt.h>


/*
 * spi_reserve(): Don't let the INT0 ISR use SPI
 *
 * This is needed because otherwise, the ISR wouldn't have a clue whether the SPI is busy in the app
 * Previously, this was implemented by setting a variable, but that had no speed advantage whatsoever
 */
#ifndef YACA_INT1
#define spi_reserve() GICR &= ~(1 << INT0)
#define spi_free() GICR |= (1 << INT0)
#else
#define spi_reserve() GICR &= ~(1 << INT1)
#define spi_free() GICR |= (1 << INT1)
#endif

#define bytewise(var, b) (((uint8_t*)&(var))[b])


tstatus _frame_transmit(Message* f);
uint8_t _read_frame(Message* f, uint8_t read_cmd);

// Optimize code size: gcc 4.3.3 inlines, which leads to additional code usage. Squeeze some additional bytes out!!
void seterror(uint8_t errmask) __attribute__((noinline)); // save ~24 bytes


#define BIOSRAM __attribute__((section (".biosram")))
//#define BIOSRAM
// section .biosram: won't be allocated in application (shared memory area - the app will use our comm routines, i.e. our buffer)
// the following variable _NEEDS_ to be static. Otherwise, avr-gcc 4.2.2 will try to write it at a wrong address from the ISR :-(
volatile Message BIOSRAM __bld_message;
volatile uint8_t BIOSRAM __err;
volatile uint8_t BIOSRAM __bld_message_used;



// seterror: safely set error flag
void seterror(uint8_t errmask) {
	uint8_t sr = SREG;
	cli();
	__err |= errmask;
	SREG = sr;
}

void yc_init() {
	__bld_message_used = 0;
	__err = 0;
	mcp2515_init();
}

// TODO: check error codes to see whether the message has caused an error
tstatus _frame_transmit(Message* f) {
	uint8_t status, i, len;

	// make sure we don't accidentally send an RTR because of an application error
	len = f->length & 0x0F;

	spi_reserve();
	// read status to determine whether we can send the message
	status = mcp2515_rdstatus(MCP2515_CMD_READ_STATUS);
	spi_free();

	// we only use one transmission buffer, TXB0
	if(bit_is_set(status, 2)) // 2 ^= TXB0CNTRL.TXREQ -> buffer busy?
		return PENDING; // if buffer is busy, we rely on the application polling us with frame_poll_transmit()

	if(f->info) { // did we even send the message? if we did and buffer empty, then we've sent it! yay!
		f->info = 0;
		return SUCCESS;
	}

	spi_reserve();
	_spi_start();
	_spi_putc(MCP2515_CMD_WRITE_TX); // dull bit-shifting to get the 32 bit variable into the crappily organized MCP2515 registers
	_spi_putc((bytewise(f->id, 3) << 3) | ((bytewise(f->id, 2) & 0xE0) >> 5)); // TXB0SIDH
	_spi_putc(((bytewise(f->id, 2) & 0x1c) << 3) | (bytewise(f->id, 2) & 0x03) | (1 << EXIDE)); // TXB0SIDL
	_spi_putc(bytewise(f->id, 1)); // TXB0EID8
	_spi_putc(bytewise(f->id, 0)); // TXB0EID0
	if(f->rtr) {
		_spi_putc((1 << RTR) | len); // TXB0DLC
	} else {
		_spi_putc(len); // TXB0DLC
		for(i = 0; i < len; i++)
			_spi_putc(f->data[i]);
	}
	_spi_stop();
	asm volatile("nop");
	_spi_start();
	_spi_putc(MCP2515_CMD_RTS | 0x01); // RTS with TXB0
	_spi_stop();
	spi_free();

	f->info = 1; // we've sent it, eh?

	return PENDING;
}

tstatus yc_transmit(Message* f) {
	return _frame_transmit(f);
}

tstatus yc_poll_transmit(Message* f) {
	return _frame_transmit(f);
}

uint8_t yc_poll_receive() {
	return __bld_message_used;
}

void yc_receive(Message* f) {
	// no cli: if we get an interrupt here, it is much too early anyway -> usual error handling in ISR
	*f = __bld_message;
	uint8_t sr = SREG;
	cli();
	__bld_message_used = 0;
	SREG = sr;
}

// _read_frame: This one is only intended to be called from an ISR as it has no SPI protection
uint8_t _read_frame(Message* f, uint8_t read_cmd) {
	uint8_t temp, i;

	_spi_start();
	_spi_putc(read_cmd);

	temp = _spi_getc();
	bytewise(f->id, 3) = temp >> 3;	// dull bit-shifting to get the crappily organized MCP2515 registers into a 32 bit variable
	bytewise(f->id, 2) = temp << 5;

	temp = _spi_getc();
	bytewise(f->id, 2) |= ((temp & 0xE0) >> 3);
	bytewise(f->id, 2) |= (temp & 0x03);

	// if not extended ID, this is an ERROR!! Recoverable, but record it.
	if(bit_is_clear(temp, IDE)) {
		seterror(YCERR_CAN_STD_FRAME);
		_spi_stop();
		return 0;
	}

	bytewise(f->id, 1) = _spi_getc();
	bytewise(f->id, 0) = _spi_getc();

	temp = _spi_getc();
	f->rtr = bit_is_set(temp, RTR);
	temp &= 0x0F;
	f->length = temp;

	for(i = 0; i < temp; i++)
		f->data[i] = _spi_getc();

	_spi_stop();

	return 1;
}

#ifndef YACA_INT1
ISR(INT0_vect) {
#else
ISR(INT1_vect) {
#endif
	uint8_t status;
	Message msg;

	status = mcp2515_rdstatus(MCP2515_CMD_RX_STATUS);

	if(bit_is_set(status, 6)) { // message in RXB0
		status = _read_frame(&msg, MCP2515_CMD_READ_RX);
	} else if(bit_is_set(status, 7)) { // message in RXB1
		status = _read_frame(&msg, MCP2515_CMD_READ_RX | 0x04);
	} else {
		// TODO: error checking and handling
		return;
	}
	if(status == 1) {
		if(++__bld_message_used > 1) {
			seterror(YCERR_MCU_RX_OVERRUN);
		}
		__bld_message = msg; // the culprit line: f*cks up avr-gcc if __bld_message isn't declared static. compare it to the address used in yc_receive!
		// NOTE: the avr-gcc crap only happens with the custom linker script with the custom .biosram section.
	}
}

void yc_close() {
	// set device into config mode, clearing all errors
	mcp2515_wrreg(CANCTRL, 0x80 | CLOCK_PRESCALER_MAGIC);
}

uint8_t yc_get_error() {
	return __err;
}

