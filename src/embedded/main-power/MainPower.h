#include "Messages.h"
#include <inttypes.h>

namespace MainPower {
/*	void HDM(SetStatus(uint8_t a, uint8_t b, uint16_t c, uint8_t d));
	void HDM(LightStatus(uint8_t a));
	void HDS(Status(uint8_t a));*/
	void HDS(HelloWorld(uint8_t a));
	void HDS(PowerStatus(uint8_t ac));
}

