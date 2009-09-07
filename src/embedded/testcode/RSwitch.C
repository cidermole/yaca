/*
 * This is an auto-generated file. Do not edit, your changes will be lost
 * the next time this file is built.
 *
 * Created:       Fri Jul 31 00:45:57 2009
 * Original file: Switch.C
 */

#include "RSwitch.h"

void __attribute__ ((naked, noinline)) RSwitch::__msg_Status(unsigned char p0) {
	asm volatile(
		"ldi r31, 0x01" "\n\t" // packstyle: 1
		"rjmp _pack_n_go" "\n\t"
	:::"r31");
}

void __attribute__ ((naked, noinline)) RSwitch::__msg_SetStatus(unsigned char p0, unsigned char p1, unsigned int p2, unsigned char p3) {
	asm volatile(
		"ldi r31, 0x13" "\n\t" // packstyle: 19
		"rjmp _pack_n_go" "\n\t"
	:::"r31");
}

