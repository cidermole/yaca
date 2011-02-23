#include "timesync.h"

int32_t vts_missing;
uint16_t vts_dist, vts_rem;
int8_t vts_sign;


filter_t _ts_filter(filter_t input) {
	static filter_t arr[SLOT_COUNT];
	static filterindex_t in = 0;
	static uint8_t init = 0;
	static mssum_t sum;
	filterindex_t i;

	if(init == 0) {
		for(i = 0; i < SLOT_COUNT; i++)
			arr[i] = input;
		sum = ((mssum_t) SLOT_COUNT) * input;
		init = 1;
	}

	sum -= arr[in];
	sum += input;
	arr[in] = input;
	if(++in == SLOT_COUNT)
		in = 0;

	return sum / SLOT_COUNT;
}

void ts_init() {
	// XXX: static variables in functions are not affected
	vts_missing = 1;
	vts_rem = 0;
	vts_sign = 0;
}

mscount_t add_ms(mscount_t a, mscount_t b) {
	mscount_t sum = a + b;
	if(sum >= TIME_MAX_MS && sum > a)
		sum -= TIME_MAX_MS;
	else if(sum < a)
		sum = add_ms(sum, MSCOUNT_MAX - TIME_MAX_MS);
	return sum;
}

int16_t ts_slot(int32_t ms, int32_t corr_ms, int32_t real_ms) {
	static int32_t last_ms = 0;

	if(last_ms - ms > 10000)
		last_ms = ms - 1000;

	vts_missing = _ts_filter(I_FACTOR * (1000 - ms + last_ms)) - (corr_ms - real_ms);
	vts_sign = vts_missing >= 0 ? 1 : -1;
	vts_missing = vts_sign == 1 ? vts_missing : -vts_missing;
	vts_missing = vts_missing == 0 ? 1 : vts_missing;
	vts_dist = (1000UL * I_FACTOR + vts_rem) / vts_missing;
	vts_rem = (1000UL * I_FACTOR + vts_rem) % vts_missing;
	last_ms = ms;

	return corr_ms - real_ms;
}

int8_t ts_tick(mscount_t ms, uint8_t reset) {
	static mscount_t next_ms = 0;
	static uint8_t next_wrap = 0;
	mscount_t i;

	if(reset)
		next_ms = 0;

	if((ms >= next_ms && !next_wrap) || (next_wrap && ms < (TIME_MAX_MS / 2) && ms >= next_ms)) {
		i = next_ms + vts_dist;
		next_wrap = (i < next_ms || i >= TIME_MAX_MS);
		next_ms = add_ms(next_ms, vts_dist);
		
		vts_dist = (((mssum_t) SLOT_LEN_MS) * I_FACTOR + vts_rem) / vts_missing;
		vts_rem = (((mssum_t) SLOT_LEN_MS) * I_FACTOR + vts_rem) % vts_missing;

		return vts_sign;
	}

	return 0;
}

