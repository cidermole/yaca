#include "Messages.h"
#include <stdint.h>

#define YC_EE_TIME_ID YE(508) // int32

namespace Time {
	void HDS(Debug(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h));
	void HDS(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags));

	void HDM(AddTimeOffset(int16_t ms));
}

