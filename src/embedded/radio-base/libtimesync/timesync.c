#include "timesync.h"

mscount_t vts_soll, vts_missing;
mscount_t vts_dist, vts_rem;
int8_t vts_sign;


mscount_t _ts_filter(mscount_t input) {
	static mscount_t arr[SLOT_COUNT];
	static filterindex_t in = 0;
	static uint8_t init = 0;
	mssum_t avg = AVG_PERIOD;
	filterindex_t i;

	if(init == 0) {
		for(i = 0; i < SLOT_COUNT; i++)
			arr[i] = SLOT_LEN_MS;
		init = 1;
	}

	avg -= arr[in];
	avg += input;
	arr[in] = input;
	if(++in == SLOT_COUNT)
		in = 0;

	return avg / SLOT_COUNT;
}

void ts_init() {
	// XXX: static variables in functions are not affected
	vts_missing = 1;
	vts_rem = 0;
	vts_sign = 1;
}

mscount_t sub_ms(mscount_t a, mscount_t b) {
	if(b > a) {
		b -= a;
		return TIME_MAX_MS - b;
	} else {
		return a - b;
	}
}

mscount_t add_ms(mscount_t a, mscount_t b) {
	mscount_t sum = a + b;
	if(sum >= TIME_MAX_MS && sum > a)
		sum -= TIME_MAX_MS;
	else if(sum < a)
		sum = add_ms(sum, MSCOUNT_MAX - TIME_MAX_MS);
	return sum;
}

void ts_slot(mscount_t ms, mscount_t corr_ms, mscount_t real_ms) {
	static mscount_t last_ms = 0;

	vts_soll = _ts_filter(sub_ms(ms, last_ms));

	vts_missing = add_ms(sub_ms(SLOT_LEN_MS, vts_soll), sub_ms(real_ms, corr_ms));

	vts_sign = vts_missing < (TIME_MAX_MS / 2) ? 1 : -1;
	if(vts_sign == -1)
		vts_missing = TIME_MAX_MS - vts_missing;
	if(vts_missing == 0)
		vts_missing = 1;

	vts_dist = (SLOT_LEN_MS + vts_rem) / vts_missing;
	vts_rem = (SLOT_LEN_MS + vts_rem) % vts_missing;

	last_ms = ms;
}

int8_t ts_tick(mscount_t ms) {
	static mscount_t next_ms = 0;
	static uint8_t next_wrap = 0;
	mscount_t i;

	if((ms >= next_ms && !next_wrap) || (next_wrap && ms < (TIME_MAX_MS / 2) && ms >= next_ms)) {
		i = next_ms + vts_dist;
		next_wrap = (i < next_ms || i >= TIME_MAX_MS);
		next_ms = add_ms(next_ms, vts_dist);
		
		vts_dist = (SLOT_LEN_MS + vts_rem) / vts_missing;
		vts_rem = (SLOT_LEN_MS + vts_rem) % vts_missing;

		return vts_sign;
	}

	return 0;
}

