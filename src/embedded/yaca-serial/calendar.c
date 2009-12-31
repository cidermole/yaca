#include "calendar.h"


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

////////////////////////////////////////////////////////////////////////////////

uint8_t day_of_week(uint16_t y, uint8_t m, uint8_t d) {
	uint8_t c_offset, y_offset, m_offset = 0;
	uint8_t i;
	// weekday = century + year + month + day, all offsets. modulo 7
	
	c_offset = (uint8_t) ((32767 - y / 100) % 4) * 2;
	y_offset = ((y % 100) + (y % 100) / 4) % 7;
	if(IS_LEAP_YEAR(y) && (m == 1 || m == 2))
		y_offset--;
	// month offset
	for(i = 1; i < m; i++) {
		m_offset += days_in_month(i, 1); // 1 -> calculation without leap years
		m_offset = m_offset % 7;
	}
	return ((c_offset + y_offset + m_offset + d) % 7);
}

