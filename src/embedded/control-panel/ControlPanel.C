#include "ControlPanel.h"
#ifndef F_CPU
#define F_CPU 14745000UL
#endif
#include <yaca.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "uart.h"
#include "../bootloader/msgdefs.h"
#include "../bootloader/eeprom.h"

uint32_t tcp_in_id, tcp_out_id, tid;

void msg_bld_check(Message *msg) {
	if(msg->id == tid && msg->data[0] == TID_BLD_ENTER)
		yc_bld_reset();
}

int main() {
	Message msg;

	uart_init((uint16_t) (((F_CPU / (16.0 * BAUDRATE) - 1) * 2 + 1) / 2));
	eeprom_read_block(&tcp_in_id, YC_EE_TCP_IN_ID, sizeof(tcp_in_id));
	eeprom_read_block(&tcp_out_id, YC_EE_TCP_OUT_ID, sizeof(tcp_out_id));
	eeprom_read_block(&tid, EE_TEMPID, sizeof(tid));

	sei();

	while(1) {
		if(yc_poll_receive()) {
			yc_receive(&msg);
			msg_bld_check(&msg);
		}
	}
}

#include "fifo.c"
#include "uart.c"

