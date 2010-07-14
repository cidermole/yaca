#ifndef CALENDAR_H
#define CALENDAR_H

#include <stdint.h>

#define IS_LEAP_YEAR(y) (((y) % 4 == 0) && (((y) % 100 != 0) || ((y) % 400 == 0)))

uint8_t days_in_month(uint8_t m, uint16_t y) {
	switch(m) {
		case 1: return 31;
		case 2:
			if(IS_LEAP_YEAR(y))
				return 29;
			else
				return 28;
			break;
		case 3: return 31;
		case 4: return 30;
		case 5: return 31;
		case 6: return 30;
		case 7: return 31;
		case 8: return 31;
		case 9: return 30;
		case 10: return 31;
		case 11: return 30;
		case 12: return 31;
		
		default: return 30;
	}
}

#endif /* CALENDAR_H */

