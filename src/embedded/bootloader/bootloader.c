#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <util/crc16.h>
#include "bootloader.h"
#include "utils.h"
#include "msgdefs.h"

// make compiler happy
#ifndef NULL
#define NULL 0
#endif

volatile uint16_t _time = 0;
uint32_t tempid;
uint8_t err_code;

void mcp2515_set_int(uint8_t);

void _error(uint8_t ec) {
	cli();
	uint8_t temp;
	if((temp = eeprom_read_byte(EE_ERR)))
		temp |= YCERR_MULTI;
	else
		temp = ec | yc_get_error();
	eeprom_write_byte(EE_ERR, temp);
	while(1);
}

/**
 * \brief Code-squeezing method for setting info = 0, id, length, data[0] and calling yc_transmit().
 * 
 * XXX: yc_transmit() return value is not checked, i.e. this method is not suitable for transmitting when the TX buffer may be full.
 * 
 **/
void txMessageWithStatusAndLength(Message *msg, uint8_t status, uint8_t length) {
	msg->id = tempid;
	msg->info = 0;
	msg->length = length;
	msg->data[0] = status;
	// we can assume the message always works out OK as we send replies in a sequential protocol
	yc_transmit(msg);
}

void txCrcError(Message *msg) {
	msg->data[0] = TID_BLD_CRC_ERR;
	txMessageWithStatusAndLength(msg, TID_BLD_CRC_ERR, 1);
}

int __attribute__((noreturn)) main() {
	Message msg;
	uint8_t state = 0;
	uint16_t timeout = BLD_TIMEOUT;
	uint16_t bldPage = 0;
	uint8_t bldByte = 0;
	uint8_t pageBuffer[BLD_PAGE_SIZE];
	uint8_t i;

	// move interrupts to bootloader section
	GICR = (1 << IVCE);
	GICR = (1 << IVSEL);
	
	err_code = 0;

	delay_ms(EEPROM_WARMUP_DELAY);
	yc_init();

	// PORTABILITY: timer will need to be adapted
	// TODO: some EEPROM config value for CPU MHz
	// 1.000.000 / 8 = 125.000 / (125 * MHz) = 1.000 ticks/sec.
	TCCR1B = (1 << CS11) | (1 << WGM12); // Timer1 presc. = 8, CTC mode
	OCR1A = (uint16_t)((125UL * F_CPU) / 1000000UL);
	TIMSK |= (1 << OCIE1A); // enable CTC interrupt

	sei();

//	eeprom_read_block(&tempid, EE_TEMPID, sizeof(tempid));
	((uint16_t *)(&tempid))[0] = eeprom_read_word(EE_TEMPID);
	((uint16_t *)(&tempid))[1] = eeprom_read_word(EE_TEMPID + 1); // save ~100 bytes

/* DEBUG: send a message

	msg.data[0] = 0x79;
	msg.id = tempid;
	msg.info = 0;
	msg.length = 1;
	i = yc_transmit(&msg);
	while(i == PENDING) {
		delay_ms(1);
		i = yc_poll_transmit(&msg);
	}
*/
	// If the Power-on Reset Flag is not set, we came from the app
	if(!(MCUCSR & (1 << PORF)))
		goto _from_app;

	// Main state machine
	while(1) {
		switch(state) {
		/*
		 * State 0
		 * Powerup state, wait BLD_TIMEOUT ms
		 */
		case 0:
			cli(); // _time read is atomic
			if(_time > timeout) {
				sei();
				bootApp();
				txCrcError(&msg); // if bootApp() returned, we have a CRC error
				state = 1;
			}
			sei();

			if(receiveValidateTempidMsg(&msg)) {
				if(msg.data[0] == TID_BLD_ENTER) {
_from_app:
					state = 1;
				}
			}
			break;

		/*
		 * State 1
		 * Bootloader entered - no timeout, wait for programming or boot
		 */
		case 1:
			if(receiveValidateTempidMsg(&msg)) {
				switch(msg.data[0]) {
				case TID_BLD_PAGESEL: // Page select
					bldPage = *((uint16_t*)(&(msg.data[1])));
					if(bldPage >= BLD_APP_PAGE_COUNT) {
						bldPage = BLD_APP_PAGE_COUNT - 1;
						err_code |= YCERR_PAGE_OVERRUN;
					}
					bldByte = 0;
					break;

				case TID_BLD_DATA: // Bootloader page data
					for(i = 1; i < msg.length && bldByte < BLD_PAGE_SIZE; i++) // copy data bytes into buffer
						pageBuffer[bldByte++] = msg.data[i];

					if(bldByte == BLD_PAGE_SIZE) { // if the whole page is loaded, flash it
						flashPage(bldPage++, pageBuffer);
						bldByte = 0;
						txMessageWithStatusAndLength(&msg, TID_BLD_PGDONE, 1);
					}
					break;

				case TID_BLD_EE_WR: // Write a byte to EEPROM
					yc_close();
					cli();

					eeprom_write_byte((uint8_t *)(*((uint16_t *)(&msg.data[1]))), msg.data[3]);

					yc_init();
					sei();
					txMessageWithStatusAndLength(&msg, TID_BLD_EE_WROK, 1);
					break;
				
				case TID_BLD_EE_RD: // Read a byte from EEPROM
					msg.data[1] = eeprom_read_byte((uint8_t *)(*((uint16_t *)(&msg.data[1]))));
					txMessageWithStatusAndLength(&msg, TID_BLD_EE_DATA, 2);
					break;

				case TID_BLD_BOOT: // Boot the app
					bootApp();
					txCrcError(&msg); // if bootApp() returned, we have a CRC error
					break;
				}

				case TID_BLD_GETSIG: // Get device signature
					msg.data[1] = SIGNATURE_0;
					msg.data[2] = SIGNATURE_1;
					msg.data[3] = SIGNATURE_2;
					txMessageWithStatusAndLength(&msg, TID_BLD_SIG, 4);
					break;
			}
			break;

		default:
			err_code |= YCERR_BLD_FSM;
			break;
		}

		if(yc_get_error() || err_code)
			_error(err_code);
	}
}

/**
 * \brief Check CRC and start application if CRC is OK, return otherwise.
 **/
void bootApp() {
	void (*fp)() = (void (*)())0;
	
	uint16_t crc = 0xFFFF;
	uint8_t* p;
	
	for(p = NULL; p < ((uint8_t *)(BLD_APP_PAGE_COUNT * BLD_PAGE_SIZE)); p++)
		crc = _crc16_update(crc, pgm_read_byte(p));
	
	if(eeprom_read_word(EE_CRC16) != crc)
		return;

	// deactivate Timer1
	cli();
	TCCR1B = 0;
	TIMSK = 0;
	// move interrupts to app section XXX the app needs to enable INT0 and jump to our implementation of INT0
	GICR = (1 << IVCE);
	GICR = (0 << IVSEL); //| (1 << INT0);
	mcp2515_set_int(1); // re-enable the external interrupt cleared by the lines above
	
	// the app only needs to enable interrupts
	fp();
}

void flashPage(uint16_t page, uint8_t* buffer) {
	uint16_t i;

	// this cli() means 3.7 - 4.5 ms of disabled interrupts, according to the data sheet (programming time)
	// 125 kbps on CAN + min. CAN frame length of (64 + 3IFS) -> 0,536 ms for a frame :(
	// "close" the CAN connection and re-open afterwards
	yc_close();
	cli();

	eeprom_busy_wait();
	boot_page_erase(((uint16_t)page) * BLD_PAGE_SIZE);
	boot_spm_busy_wait();

	for(i = 0; i < BLD_PAGE_SIZE; i += 2) {
		uint16_t w = *buffer++; // setup a word
		w += (*buffer++) << 8;

		boot_page_fill(((uint16_t)page) * BLD_PAGE_SIZE + i, w);
	}
	boot_page_write(((uint16_t)page) * BLD_PAGE_SIZE);
	boot_spm_busy_wait();
	boot_rww_enable();

	yc_init(); // re-init the MCP2515
	sei();
}

uint8_t receiveValidateTempidMsg(Message* msg) {
	if(!yc_poll_receive())
		return 0;
	yc_receive(msg);

	return (msg->id == tempid && msg->length != 0);
}

/*
ISR(TIMER1_COMPA_vect) {
	_time++;
}
*/

// Let's squeeze bytes! *yay*

void TIMER1_COMPA_vect() __attribute__((naked, signal, __INTR_ATTRS));

ISR(TIMER1_COMPA_vect) {
	asm volatile(
		"push r28" "\n\t"
		"push r29" "\n\t"
		"push r30" "\n\t"
		"in r30, __SREG__" "\n\t"
		
		"lds r28, _time" "\n\t"
		"lds r29, _time + 1" "\n\t"
		"adiw r28, 1" "\n\t"
		"sts _time, r28" "\n\t"
		"sts _time + 1, r29" "\n\t"
		
		"out __SREG__, r30" "\n\t"
		"pop r30" "\n\t"
		"pop r29" "\n\t"
		"pop r28" "\n\t"
		"reti" "\n\t"
	);
}

