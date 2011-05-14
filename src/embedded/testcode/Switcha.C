#include "Switcha.h"
#include "RSwitcha.h"
#include <avr/interrupt.h>
#include <yaca.h>

int main() {
	sei();

	yc_send(Switcha, Test(0xDE));

	while(1) {
		yc_dispatch_auto();
	}
	return 0;
}
