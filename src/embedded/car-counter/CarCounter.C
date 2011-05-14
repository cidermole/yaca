#include "CarCounter.h"
#include "RCarCounter.h"
#include <avr/interrupt.h>
#include <yaca.h>

uint32_t count = 0;

void DR(Count()) {
	yc_prepare_ee(YC_EE_COUNT_ID);
	yc_send(CarCounter, Count(count));
}

int main() {
	sei();

	yc_status(Count);

	while(1) {
		yc_dispatch_auto();
	}
	return 0;
}

