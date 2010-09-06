#include "Messages.h"
#include <inttypes.h>

#define YC_EE_TEMPSTATUS_ID YE(508) // int32

namespace RadioBase {
	void HDS(TempStatus(int16_t temp_deci, uint16_t millivolt));
	void HDR(TempStatus());

	void HDM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags));
}

