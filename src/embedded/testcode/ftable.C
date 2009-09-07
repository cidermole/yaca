/*
 * This is an auto-generated file. Do not edit, your changes will be lost
 * the next time this file is built.
 *
 * Created:       Fri Jul 31 00:45:57 2009
 * Original file: Switch.C
 */

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "Switch.h"

extern "C" {
	typedef struct {
		uint8_t packstyle;
		uint8_t flags;
		void *fp;
	} fpt_t;
}

extern fpt_t fpt[] PROGMEM;
extern uint8_t fpt_size PROGMEM;

fpt_t fpt[] = {
	{0x13, 0x00, (void *)Switch::__msg_SetStatus},
	{0x01, 0x00, (void *)Switch::__msg_LightStatus}
};
uint8_t fpt_size = 2;

