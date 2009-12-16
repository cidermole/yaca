#include "Messages.h"
#include <inttypes.h>

#define YC_EE_POWERSTATUS_ID YE(508) // int32

namespace MainPower {
	void HDR(PowerStatus(uint8_t ac_power, uint16_t adc_voltage, uint16_t adc_current));
}

