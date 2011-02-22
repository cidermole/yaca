#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "timesync.h"

double rnd() {
	return ((double) rand()) / RAND_MAX;
}

#define JITTER 30

int main() {
	int real_time, next_stamp, next_stamp_real, fb;
	double val = 0, input = 0, soll, soll2; // soll: time feedback with jitter
	double time_counter = 0, corrected_counter = 0;
	double delayed_time_counter;
	double DEVIATION = 0.96;

	srand(0);
	ts_init();

	next_stamp = JITTER - rnd() * JITTER;
	next_stamp_real = 0;
	for(real_time = 0; real_time < 2000 * 1000; real_time++, time_counter += DEVIATION, corrected_counter += DEVIATION) {
		if(real_time == next_stamp) {
			delayed_time_counter = time_counter;
			next_stamp_real += 1000;
			next_stamp = next_stamp_real + (JITTER / 2) - rnd() * JITTER;
		}
		if(real_time == next_stamp_real - 500)
			ts_slot(((int) delayed_time_counter) % 60000, ((int) corrected_counter) % 60000, ((int) (next_stamp_real - 500)) % 60000);

		fb = ts_tick(((int) time_counter) % 60000);
		if(fb == 1)
			corrected_counter += DEVIATION;
		else if(fb == -1)
			corrected_counter -= DEVIATION;

		printf("%d;%d;%d\n", real_time, /*(int) soll*/ real_time, (int) corrected_counter);
		fprintf(stderr, "%d;%d\n", real_time, (int) corrected_counter - real_time);

/*		if(real_time < 1000000)
			DEVIATION += 0.2 / 10000000;*/
	}

	return 0;
}

