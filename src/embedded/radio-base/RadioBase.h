#include "Messages.h"
#include <inttypes.h>

#define YC_EE_TEMPSTATUS_ID YE(508) // int32

namespace RadioBase {
	void HDS(TempStatus(int16_t temp_deci, uint16_t millivolt));
	void HDR(TempStatus());
}

