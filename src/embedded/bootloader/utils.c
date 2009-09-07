#include <inttypes.h>
#include <util/delay.h>
#include "utils.h"

void delay_ms(uint16_t t) {
	uint16_t i;
	for(i = 0; i < t; i++)
		_delay_ms(1);
}

