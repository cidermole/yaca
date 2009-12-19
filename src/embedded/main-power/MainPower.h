#include "Messages.h"
#include <inttypes.h>

#define YC_EE_POWERSTATUS_ID YE(508) // int32

namespace MainPower {
	void HDS(PowerStatus(uint8_t ac_power, uint16_t adc_voltage, uint16_t adc_current, uint8_t t_high, uint16_t t));
	void HDR(PowerStatus());
	
	void HDS(Debug1(uint32_t value));
	void HDS(Debug2(uint32_t value));
}

