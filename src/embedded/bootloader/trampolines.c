#include <avr/interrupt.h>

void __attribute__((naked, noinline, section (".biostrampolines"))) __trampolines() {
	asm volatile(
		"rjmp yc_init" "\n\t"
		"rjmp yc_transmit" "\n\t"
		"rjmp yc_poll_transmit" "\n\t"
		"rjmp yc_poll_receive" "\n\t"
		"rjmp yc_receive" "\n\t"
		"rjmp yc_get_error" "\n\t"
		"rjmp yc_close" "\n\t"
	);
}

