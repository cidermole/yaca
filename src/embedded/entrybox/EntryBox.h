#include "Messages.h"
#include <stdint.h>

#define YC_EE_JOULESTATUS_ID YE(508) // int32
#define YC_EE_POWERSTATUS_ID YE(504) // int32

namespace EntryBox {
	void HDS(JouleStatus(uint32_t joule_battery, uint32_t joule_solar));
	void HDR(JouleStatus());
	void HDS(PowerStatus(uint16_t vbat, int16_t ibat, uint16_t isol));
	void HDR(PowerStatus());

	//void HDM(SetCount(uint32_t count));
}

