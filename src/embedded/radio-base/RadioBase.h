#include "Messages.h"
#include <inttypes.h>

#define YC_EE_TEMPSTATUS_ID YE(508) // int32

namespace RadioBase {
	void HDS(TempStatus(int16_t temp_deci, uint16_t millivolt));
	void HDR(TempStatus());

	void HDS(Debug(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h));
}

