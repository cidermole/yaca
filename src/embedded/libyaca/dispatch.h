#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include <avr/pgmspace.h>
#include <inttypes.h>
#include "yaca.h"

// FIXME: why do we need this #define goop??
void _unpack(uint8_t* pdata, uint8_t packstyle); // ASM: load message data in regs; call target function immediately afterwards

// void unpack(Message* pdata, uint8_t packstyle);
#define unpack(a, b) _unpack((uint8_t*)(&(a->data[0])), b)

void _pack_n_go(); // ASM: needs _global_packstyle (r31) set correctly, calls _take_off

void _take_off(Message* m); // needs _global_canid set correctly (via yc_prepare())

#endif /* _DISPATCH_H_ */

