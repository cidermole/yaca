#include "Messages.h"
#include <inttypes.h>

namespace Switcha {
	void HDM(SetLed(uint8_t a));
	void HDS(LedStatus(uint8_t a));
	void HDR(LedStatus());
}

