#include "RadioBase.h"
#include "RRadioBase.h"
#include <yaca.h>

uint8_t _n = 0;

void DR(Wtf()) {
	yc_prepare(406);
	yc_send(RadioBase, Wtf(_n++));
}

