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

void msg_bld_check(Message *msg) {
	if(msg->id == tid && msg->data[0] == TID_BLD_ENTER)
		yc_bld_reset();
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
		while((c = uart_getc_nowait()) != -1 && msg_out.length < 8)
			msg_out.data[msg_out.length++] = c;
		if(msg_out.length) {
			if(yc_transmit(&msg_out) == PENDING)
				pending = 1;
		}
	}
}

#include "fifo.c"
#include "uart.c"

