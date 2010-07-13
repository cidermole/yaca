#include "Messages.h"
#include <inttypes.h>

#define YC_EE_PHSTATUS_ID YE(508) // int32
#define YC_EE_PUMP_FROM_HOUR YE(507) // uint8
#define YC_EE_PUMP_TO_HOUR YE(506) // uint8

namespace PoolControl {
	void HDS(PhStatus(uint16_t ph_centi));
	void HDR(PhStatus());

	void HDM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags));
	void HDM(SetMode(uint8_t mode));
}

