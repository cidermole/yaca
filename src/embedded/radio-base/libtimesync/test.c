#include "timesync.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define JITTER_MS 30 // +/-
#define JITTER_SAFE_MAX 2000 // safety margin which jitter must never exceed

double _mscounter;
const double ms_increment = 0.99;

uint16_t mscounter() {
	return _mscounter;
}

int main() {
	uint16_t t, nr;
	int8_t tick;
	int i;

	srand(2);
	ts_init();

	_mscounter = JITTER_MS;

	for(i = 0; i < 10; i++) {
		nr = 10000 + (rand() % (2 * JITTER_MS)) - JITTER_MS;
		for(t = 0; t < 60000; t++) {
			if(t % 1000 == 0)
				//fprintf(stderr, "T M D: %d %d %d\n", (int) t, (int) mscounter(), (int) (t - mscounter()));
				//fprintf(stderr, "%d;%d\n", i * 60000 + ((int) t), i * 60000 + ((int) mscounter()) + (t == 0 && mscounter() > 20000 ? -60000 : 0));
				fprintf(stderr, "%d;%d\n", i * 60 + ((int) t / 1000), (int) ((mscounter() - t) > 30000 || (mscounter() - t) < -30000) ? 0 : (mscounter() - t));
			tick = 0;
			if(t == nr) {
				printf("ts_slot(%d)\n", (int) mscounter());
				ts_slot(mscounter());
				nr = ((((nr + JITTER_SAFE_MAX) / 10000) + 1) * 10000  + (rand() % (2 * JITTER_MS)) - JITTER_MS) % 60000;
			} else {
				tick = ts_tick(mscounter());
			}

			if(tick == 1)
				_mscounter += ms_increment;
			if(tick != -1)
				_mscounter += ms_increment;
			if(mscounter() >= 60000)
				_mscounter -= 60000;
		}
	}

	return 0;
}


/*
extern uint16_t tick_dist, next_tick_ms;
extern int16_t tick_rem;
extern int8_t tick_sign;

void dump() {
	printf("tick_dist: %d\n", (int) tick_dist);
	printf("tick_sign: %d\n", (int) tick_sign);
	printf("tick_rem: %d\n", (int) tick_rem);
	printf("next_tick_ms: %d\n", (int) next_tick_ms);
	printf("\n");
}


int main() {
	printf("ts_init()\n");
	ts_init();
	dump();
	printf("ts_slot()\n");
	ts_slot(14327);
	dump();
	printf("ts_tick()\n");
	ts_tick(14465);
	dump();
	return 0;
}
*/

