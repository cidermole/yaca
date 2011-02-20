#include "timesync.h"
#include <stdio.h>


extern uint16_t tick_dist, next_tick_ms;
extern int16_t tick_rem;

void dump() {
	printf("tick_dist: %d\n", (int) tick_dist);
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

