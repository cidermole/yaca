#include "ControlPanel.h"
#ifndef F_CPU
#define F_CPU 14745000UL
#endif
#include <yaca.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

uint32_t tcp_in_id, tcp_out_id;

void enter_bootloader_hook() {
	cli();
	yc_bld_reset();
}

int main() {
	sei();
	eeprom_read_block(&tcp_in_id, YC_EE_TCP_IN_ID, sizeof(tcp_in_id));
	eeprom_read_block(&tcp_out_id, YC_EE_TCP_OUT_ID, sizeof(tcp_out_id));

	while(1) {
	}
}

