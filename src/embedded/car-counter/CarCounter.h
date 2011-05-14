#include "Messages.h"
#include <stdint.h>

#define YC_EE_COUNT_ID YE(508) // int32

namespace CarCounter {
	void HDS(Count(uint32_t count));
	void HDR(Count());

	void HDM(SetCount(uint32_t count));
}

