#include "Messages.h"
#include <inttypes.h>

#define YC_EE_PHSTATUS_ID YE(508) // int32
#define YC_EE_TEMPSTATUS_ID YE(504) // int32
#define YC_EE_PUMP_FROM_HOUR YE(503) // int8
#define YC_EE_PUMP_TO_HOUR YE(502) // int8
#define YC_EE_RELAYSTATUS_ID YE(498) // int32
#define YC_EE_PH_OFFSET YE(496) // int16

namespace PoolControl {
	void HDS(PhStatus(uint16_t ph_centi));
	void HDR(PhStatus());
	void HDS(TempStatus(int16_t temp_deci));
	void HDR(TempStatus());
	void HDS(RelayStatus(uint8_t pump_on));
	void HDR(RelayStatus());

	void HDM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags));
	void HDM(SetMode(uint8_t mode));
	void HDM(SetRelay(uint8_t status));
}

