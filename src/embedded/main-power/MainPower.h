#include "Messages.h"
#include <inttypes.h>

namespace MainPower {
	void HDS(PowerStatus(uint8_t ac_power, uint16_t adc_voltage, uint16_t adc_current));
}

