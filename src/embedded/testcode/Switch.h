#include "Messages.h"
#include <inttypes.h>

#define YC_EE_STATUSID1 YE(10) // int32
#define YC_EE_STATUSID2 YE(14) // int32

namespace Switch {
	void HDM(SetStatus(uint8_t a, uint8_t b, uint16_t c, uint8_t d));
	void HDM(LightStatus(uint8_t a));
	void HDS(Status(uint8_t a));
}

