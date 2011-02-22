#include "timesync.h"

#define STEPS 120

uint16_t vts_soll, vts_missing;
uint16_t vts_dist, vts_rem;
int8_t vts_sign;


uint16_t _ts_filter(uint16_t input) {
	static uint16_t arr[STEPS];
	uint32_t avg = 1000UL*STEPS;
	static uint8_t in = 0, init = 0;
	uint8_t i;

	if(init == 0) {
		for(i = 0; i < STEPS; i++)
			arr[i] = 1000;
		init = 1;
	}

	avg -= arr[in];
	avg += input;
	arr[in] = input;
	if(++in == STEPS)
		in = 0;

	return avg / STEPS;
}

void ts_init() {
	// XXX: static variables in functions are not affected
	vts_missing = 1;
	vts_rem = 0;
	vts_sign = 1;
}

uint16_t sub_ms(uint16_t a, uint16_t b) {
	if(b > a) {
		b -= a;
		return 60000 - b;
	} else {
		return a - b;
	}
}

uint16_t add_ms(uint16_t a, uint16_t b) {
	uint16_t sum = a + b;
	if(sum >= 60000 && sum > a)
		sum -= 60000;
	else if(sum < a)
		sum = add_ms(sum, UINT16_MAX - 60000);
	return sum;
}

void ts_slot(uint16_t ms, uint16_t corr_ms, uint16_t real_ms) {
	static uint16_t last_ms = 0;

	vts_soll = _ts_filter(sub_ms(ms, last_ms));

	vts_missing = add_ms(sub_ms(1000, vts_soll), sub_ms(real_ms, corr_ms));

	vts_sign = vts_missing < 30000 ? 1 : -1;
	if(vts_sign == -1)
		vts_missing = 60000 - vts_missing;
	if(vts_missing == 0)
		vts_missing = 1;

	vts_dist = (1000 + vts_rem) / vts_missing;
	vts_rem = (1000 + vts_rem) % vts_missing;

	last_ms = ms;
}

int8_t ts_tick(uint16_t ms) {
	static uint16_t next_ms = 0, next_wrap = 0;
	uint16_t i;

	if((ms >= next_ms && !next_wrap) || (next_wrap && ms < 30000 && ms >= next_ms)) {
		i = next_ms + vts_dist;
		next_wrap = (i < next_ms || i >= 60000);
		next_ms = add_ms(next_ms, vts_dist);
		
		vts_dist = (1000 + vts_rem) / vts_missing;
		vts_rem = (1000 + vts_rem) % vts_missing;

		return vts_sign;
	}

	return 0;
}
