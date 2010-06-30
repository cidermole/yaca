#include "Messages.h"
#include <inttypes.h>

namespace PoolControl {
	void HDM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags));
	void HDM(SetMode(uint8_t mode));
}

