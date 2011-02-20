#ifndef TIMESYNC_H
#define TIMESYNC_H

#include <stdint.h>


int8_t ts_tick(uint16_t ms);
void ts_slot(uint16_t ms);


#endif /* TIMESYNC_H */

