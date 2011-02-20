#include "timesync.h"
#include <string.h>

ringbuf_t drift_rb[SLOT_COUNT];
rb_cnt_t drift_rb_i;
rb_sum_t drift_sum;
mscount_t tick_dist;
uint8_t tick_sign;
rb_sum_t tick_rem;
mscount_t next_tick_ms;
uint8_t next_tick_wrap;
ringbuf_t added_ticks;


void _ts_calculate_tick_dist();

void ts_init() {
	memset(drift_rb, 0, sizeof(drift_rb));
	drift_rb[0] = 1;
	drift_rb_i = 0;
	drift_sum = 1; // will be a divisor
	tick_dist = MSCOUNT_MAX;
	tick_rem = 0;
	tick_sign = 1; // +
	added_ticks = 0;
	_ts_calculate_tick_dist();
}

mscount_t _ts_handle_wraparound(mscount_t sum, mscount_t orig) {
	if(sum >= TIME_MAX_MS && sum > orig) { // TIME_MAX_MS wraparound?
		sum -= TIME_MAX_MS;
	} else if(sum < orig) { // integer wraparound?
		orig = sum;
		sum += MSCOUNT_MAX - TIME_MAX_MS; // add the int wraparound
		return _ts_handle_wraparound(sum, orig);
	}
	return sum;
}

// calculate new tick distance, remainder, sign and next_tick
void _ts_calculate_tick_dist() {
	int32_t td;
	mscount_t nt;

	td = (AVG_PERIOD + tick_rem) / drift_sum;
	tick_rem = (AVG_PERIOD + tick_rem) % drift_sum;

	tick_sign = (td >= 0);
	if(!tick_sign)
		td = -td;

	if(td > MSCOUNT_MAX)
		td = MSCOUNT_MAX;

	tick_dist = td;


	// calculate next_tick_ms and wrap
	nt = next_tick_ms + tick_dist;
	next_tick_wrap = ((nt < next_tick_ms) || (nt >= TIME_MAX_MS));
	next_tick_ms = _ts_handle_wraparound(nt, next_tick_ms);
}

int8_t ts_tick(mscount_t ms) {
	// time for next tick and no wraparound expected, or
	// wraparound expected, wraparound happened and time for next tick?
	if((ms >= next_tick_ms && !next_tick_wrap) || (next_tick_wrap && ms < TIME_MAX_MS / 2 && ms >= next_tick_ms)) {
		_ts_calculate_tick_dist();
		added_ticks += tick_sign;

		if(tick_sign)
			return 1;
		else
			return -1;
	} else {
		return 0;
	}
}

void ts_slot(mscount_t ms) {
	ringbuf_t ms_mod;
	ringbuf_t slot_drift;

	// calculate slot drift
	ms_mod = ms % SLOT_LEN_MS;
	if(ms_mod < SLOT_LEN_MS / 2)
		ms_mod = -ms_mod;
	else
		ms_mod = SLOT_LEN_MS - ms_mod;
	slot_drift = added_ticks + ms_mod;

	// update drift sum (used for averaging) and drift ring buffer
	drift_sum -= drift_rb[drift_rb_i];
	drift_sum += slot_drift;
	drift_rb[drift_rb_i] = slot_drift;

	if(drift_sum == 0) { // make sure we never divide by 0
		drift_sum = 1;
		drift_rb[drift_rb_i]++;
	}

	if(++drift_rb_i == SLOT_COUNT)
		drift_rb_i = 0;


	// recalculate tick distance
	next_tick_ms = ms;
	tick_rem = 0;
	_ts_calculate_tick_dist();
	added_ticks = 0;
}

