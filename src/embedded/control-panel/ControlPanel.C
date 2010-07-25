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

uint32_t tcp_in_id, tid;
volatile uint8_t _timer;
uint8_t _count = 0;

void msg_bld_check(Message *msg) {
	if(msg->id == tid && msg->data[0] == TID_BLD_ENTER) {
		TIMSK = 0;
		TCCR1A = 0;
		TCCR1B = 0;
		yc_bld_reset();
	}
	yc_dispatch_auto(); // we won't get here. needed for linking of libyaca and consequently the INT0 ISR
}

int main() {
	Message msg_in, msg_out;
	uint8_t i, pending = 0;
	int c;

	uart_init((uint16_t) (((F_CPU / (16.0 * BAUDRATE) - 1) * 2 + 1) / 2));
	eeprom_read_block(&tcp_in_id, YC_EE_TCP_IN_ID, sizeof(tcp_in_id));
	eeprom_read_block(&msg_out.id, YC_EE_TCP_OUT_ID, sizeof(msg_out.id));
	eeprom_read_block(&tid, EE_TEMPID, sizeof(tid));
	msg_out.rtr = 0;

	PORTD |= (1 << PD0); // enable pull-up on RxD

	TCCR1B = (1 << WGM12) | (1 << CS11); // CTC, prescaler 8
	OCR1A = 1843; // 14.745 MHz / (8 * 1843) = ~ 1 kHz (1 ms period)
	TIMSK = (1 << OCIE1A); // enable output-compare interrupt

	sei();

	while(1) {
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
				continue;
		}

		msg_out.length = 0;
		msg_out.info = 0;

		if(fifo2_read.count < 8 && fifo2_read.count > 0) {
			if(fifo2_read.count > _count) {
				_count = fifo2_read.count;
				_timer = 0;
			}
			if(_timer < 5) // 5 ms timeout for aggregating messages
				continue;
		}

		while((c = uart_getc_nowait()) != -1 && msg_out.length < 8)
			msg_out.data[msg_out.length++] = c;
		if(msg_out.length) {
			if(yc_transmit(&msg_out) == PENDING)
				pending = 1;
		}
	}
}

ISR(TIMER1_COMPA_vect) {
	_timer++;
}

#include "fifo.c"
#include "uart.c"

