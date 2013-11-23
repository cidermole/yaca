#include "Messages.h"
#include <stdint.h>

#define YC_EE_JOULESTATUS_ID YE(508) // int32
#define YC_EE_POWERSTATUS_ID YE(504) // int32
#define YC_EE_COUNT_ID YE(500) // int32

namespace EntryBox {
	void HDS(JouleStatus(uint32_t joule_battery, uint32_t joule_solar));
	void HDR(JouleStatus());
	void HDS(PowerStatus(uint16_t vbat, int16_t ibat, uint16_t isol));
	void HDR(PowerStatus());
	void HDS(Count(uint32_t count));
	void HDR(Count());

	void HDM(SetCount(uint32_t count));
	void HDM(SetDummy(uint8_t status));
	void HDM(SetCharger(uint8_t status));
	void HDM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags));
}

