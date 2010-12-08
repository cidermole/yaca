#include <stdio.h>
#include <stdint.h>

uint8_t delay_big = 0, delay_small = 0;

#define X86_TESTING
#include "synctime.c"


double factor = 1.0;
int result = 0;

int16_t print() {
	int real_delay = (int) (factor * ((delay_big * 256) + delay_small));
	printf("delay_big = %d, delay_small = %d, equates to %d ms, real %d ms\n\n", (int) delay_big, (int) delay_small, (int) ((delay_big * 256) + delay_small), real_delay);
	return (int16_t) (60000 - real_delay);
}

// time_feedback = soll - ist

void test(double fac) {
	int16_t del, i = 0;

	factor = fac;
	printf("Testing sync_time() with factor = %1.02lf, first two calls hardcoded\n", factor);
	sync_time(20000); print(); // tx 20 s too early
	sync_time(-13000); del = print(); // tx 13 s too late (test insync_max_feedback)

	while((del > 10 || del < -10) && i++ < 10) {
		sync_time(del);
		del = print();
	}
	if(i >= 10) {
		printf("!!!!!!!!\n!!!!!!!! Sync count limit of 10 reached while testing!\n!!!!!!!!\n\n");
		result = 1;
	}

	printf("\n****************************************************************************************\n");
}

int main() {
	uint16_t soll, init, i;

	printf("Manual testing sync_time() with factor = %1.02lf\n", factor);
	sync_time(-29000); print(); // tx 29 s too late
	soll = init = ((delay_big * 256) + delay_small);

	sync_time(3000); print(); // tx 3 s too early
	soll = soll + 3000;
	if(soll != ((delay_big * 256) + delay_small)) {
		printf("!!!!!!!!\n!!!!!!!! delay + time_feedback is wrong!\n!!!!!!!!\n\n");
		result = 1;
	}

	sync_time(-500); print(); // tx 0.5 s too late
	soll = soll - 500;
	if(soll != ((delay_big * 256) + delay_small)) {
		printf("!!!!!!!!\n!!!!!!!! delay + time_feedback is wrong!\n!!!!!!!!\n\n");
		result = 1;
	}
	printf("\n****************************************************************************************\n");

	test(1.04);
	test(0.95);

	printf("Non-standard (out of datasheet limits) tests:\n");
	test(1.09);
	test(0.92);

	printf("Overflow check test (slowly approaching overflow):\n");
	sync_time(20000); // tx 20 s too early (reset sync)
	for(i = 0; i < 7; i++) {
		sync_time(1000); // tx 1 s too early (slowly increase sync towards overflow)
		print();
	}
	if(((delay_big * 256) + delay_small) != init) {
		printf("!!!!!!!!\n!!!!!!!! overflow handling is incorrect!\n!!!!!!!!\n\n");
		result = 1;
	}

	if(result == 0) {
		printf("All automatic tests passed.\n\n");
	}
	return result;
}

