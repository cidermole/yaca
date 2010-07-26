#include "ControlPanel.h"
#ifndef F_CPU
#define F_CPU 14745000UL
#endif
#include <yaca.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "uart.h"
#include "../bootloader/msgdefs.h"
#include "../bootloader/eeprom.h"
#include "one-wire.h"
#include <limits.h>

#define DS18B20 // using DS18B20 with more resolution than DS18S20

uint32_t tcp_in_id, tcp_out_id, temp_id, tid;
volatile uint8_t _timer;
volatile uint16_t _conv_timer = 1001;
uint8_t _count = 0, pending = 0;
Message msg_in, msg_out;

void msg_bld_check(Message *msg) {
	if(msg->id == tid && msg->data[0] == TID_BLD_ENTER) {
		TIMSK = 0;
		TCCR1A = 0;
		TCCR1B = 0;
		yc_bld_reset();
	}
	yc_dispatch_auto(); // we won't get here. needed for linking of libyaca and consequently the INT0 ISR
}

void check_and_receive() {
	uint8_t i;
	int c;

	if(yc_poll_receive()) {
		yc_receive(&msg_in);
		msg_bld_check(&msg_in);
		if(msg_in.id == tcp_in_id) {
			for(i = 0; i < msg_in.length; i++)
				uart_putc(msg_in.data[i]);
		}

		// much time has passed... don't busy-wait
		if(pending)
			pending = 2;
	}

	// if we didn't receive something in the meantime, let some time pass
	if(pending == 1)
		_delay_us(100);

	if(pending) {
		if(yc_poll_transmit(&msg_out) == SUCCESS)
			pending = 0;
		else
			return;
	}

	msg_out.length = 0;
	msg_out.info = 0;
	msg_out.id = tcp_out_id;

	if(fifo2_read.count < 8 && fifo2_read.count > 0) {
		if(fifo2_read.count > _count) {
			_count = fifo2_read.count;
			_timer = 0;
		}
		if(_timer < 50) // ms timeout for aggregating messages
			return;
	}

	while((c = uart_getc_nowait()) != -1 && msg_out.length < 8)
		msg_out.data[msg_out.length++] = c;
	if(msg_out.length) {
		if(yc_transmit(&msg_out) == PENDING)
			pending = 1;
	}
}

int16_t measure_temp() {
	uint8_t data[9], i;
	int16_t temp_value;

	ow_check(); // ow_check() will not return 0 atm
/*	if(!ow_check()) {
		loopdelay_ms(800);
		return;
	}*/

	cli();
	if(_conv_timer > 1000) { // start conversion
		_conv_timer = 0;
		sei();
		ow_write(OW_SKIP_ROM);
		ow_write(OW_CONVERT_T, OW_PULL);
		return INT_MIN;
	} else if(_conv_timer > 800) {
		_conv_timer = 1001;
		sei();
	} else {
		sei();
		return INT_MIN;
	}

	ow_release();
	ow_check(); // same as above
/*	if(!ow_check())
		return;*/

	ow_write(OW_SKIP_ROM);
	ow_write(OW_READ_SCRATCHPAD);

	for(i = 0; i < 9; i++)
		data[i] = ow_read();

	temp_value = data[0] | (((int16_t) data[1]) << 8);
#ifdef DS18B20
	temp_value = (temp_value * 10) / 16;
#else
	temp_value *= 5; // 0.5 °C steps
	// TODO: exact temp measurement with remainder
#endif

	return temp_value;
}

int main() {
	int16_t temp = -200, _temp;
	uint8_t temp_pending = 0;

	uart_init((uint16_t) (((F_CPU / (16.0 * BAUDRATE) - 1) * 2 + 1) / 2));
	eeprom_read_block(&tcp_in_id, YC_EE_TCP_IN_ID, sizeof(tcp_in_id));
	eeprom_read_block(&tcp_out_id, YC_EE_TCP_OUT_ID, sizeof(tcp_out_id));
	eeprom_read_block(&temp_id, YC_EE_TEMPSTATUS_ID, sizeof(temp_id));
	eeprom_read_block(&tid, EE_TEMPID, sizeof(tid));
	msg_out.rtr = 0;

	PORTD |= (1 << PD0); // enable pull-up on RxD

	TCCR1B = (1 << WGM12) | (1 << CS11); // CTC, prescaler 8
	OCR1A = 1843; // 14.745 MHz / (8 * 1843) = ~ 1 kHz (1 ms period)
	TIMSK = (1 << OCIE1A); // enable output-compare interrupt

	sei();

	while(1) {
		check_and_receive();
		if((_temp = measure_temp()) != INT_MIN) { // if a temp value is here...
			temp = _temp;
			temp_pending = 1;
		}
		if(!pending && temp_pending) {
			temp_pending = 0;
			msg_out.id = temp_id;
			msg_out.length = 2;
			msg_out.info = 0;
			msg_out.data[0] = (uint8_t) (temp >> 8);
			msg_out.data[1] = (uint8_t) temp;
			if(yc_transmit(&msg_out) == PENDING)
				pending = 1;
		}
	}
}

ISR(TIMER1_COMPA_vect) {
	_timer++;
	_conv_timer++;
}

#include "fifo.c"
#include "uart.c"

