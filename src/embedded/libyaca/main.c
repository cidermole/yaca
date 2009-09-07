#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include "yaca.h"

#include "../bootloader/eeprom.h"

int __attribute__ ((weak)) main(void);
#ifdef YACA_INT1
void __attribute__ ((naked, signal, __INTR_ATTRS)) INT1_vect(void);
#else
void __attribute__ ((naked, signal, __INTR_ATTRS)) INT0_vect(void);
#endif
void __attribute__ ((weak)) init(void);

extern fpt_t fpt[] PROGMEM;
extern uint8_t fpt_size PROGMEM;

volatile uint16_t _time = 0;

#ifdef YACA_INT1
void INT1_vect(void) {
//ISR(INT1_vect) {
	asm volatile("rjmp __bld_int1");
}
#else
void INT0_vect(void) {
//ISR(INT0_vect) {
	asm volatile("rjmp __bld_int0");
}
#endif

int main() {
	Message in;

	if(init)
		init();

	sei();

	while(1) {
		if(yc_poll_receive()) {
			yc_receive(&in);
			yc_dispatch(&in, EE_IDTABLE, (void **)fpt, (uint8_t *)&fpt_size);
		}
	}
	return 0;
}

