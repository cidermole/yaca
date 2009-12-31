#include "Messages.h"
#include <inttypes.h>

#define YC_EE_LEDSTATUS_ID YE(508) // int32

namespace Switcha {
	void HDM(SetLed(uint8_t a));
	void HDS(LedStatus(uint8_t a));
	void HDR(LedStatus());
}

