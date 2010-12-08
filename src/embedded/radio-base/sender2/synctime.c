#include <stdint.h>
#ifndef X86_TESTING
#include <util/delay.h>
#endif /* X86_TESTING */
#include "main.h"

extern uint8_t delay_small, delay_big;

void sync_time(int16_t time_feedback) {
	uint16_t delay;
	int16_t new_delay_small;
	uint8_t new_delay_big;
	static uint8_t synced_before = 0;

#ifdef X86_TESTING
	printf("syncing time_feedback %d\n", (int) time_feedback);
#endif /* X86_TESTING */

	if(!synced_before || time_feedback > INSYNC_MAX_FEEDBACK_DEVIATION || time_feedback < -INSYNC_MAX_FEEDBACK_DEVIATION) {
		if(time_feedback < 0)
			delay = 60000 - (uint16_t)(-time_feedback);
		else
			delay = (uint16_t) time_feedback;

#ifndef X86_TESTING
		// busy-wait the first time
		while(delay > 0)
			_delay_ms(1);
#else
		printf("busywait of %d ms\n", (int) delay);
#endif /* X86_TESTING */

		// set default delay values, approx. 1 min.
		delay_big = 230; // 230 * 256 = 58.880 ms
		delay_small = 220; // + 220 ms + measurements etc. (900 ms) = 60.000 ms
	} else {
		// fine tuning
		new_delay_small = delay_small;
		new_delay_small += time_feedback;

		new_delay_big = (uint8_t) (((int8_t) delay_big) + (int8_t) (new_delay_small / 256)); // add to or borrow from from delay_big
		new_delay_small %= 256;
		if(new_delay_small < 0) {
			new_delay_big--;
			new_delay_small += 256;
		}

		if(new_delay_big <= INSYNC_MAX_FEEDBACK_DEVIATION / 256 && delay_big > (0xFFFF - INSYNC_MAX_FEEDBACK_DEVIATION) / 256) {
#ifdef X86_TESTING
			printf("new_delay_big overflow, restarting sync.\n");
#endif /* X86_TESTING */
			// overflow
			synced_before = 0;
			sync_time(time_feedback);
		} else {
			delay_big = new_delay_big;
			delay_small = (uint8_t) new_delay_small;
		}
	}

	synced_before = 1;
}

