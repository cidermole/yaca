#include "Switch.h"
#include "RSwitch.h"
//#include <yaca.h>
#include <avr/io.h>
#ifndef F_CPU
#define F_CPU 2000000UL
#endif
#include <util/delay.h>

extern "C" {
typedef struct {
	uint8_t info; ///< was frame_id, local id of frame
	uint32_t id;
	uint8_t rtr; ///< Whether this is a remote transmission request
	uint8_t length;
	uint8_t data[8];
} Message;

void yc_prepare(uint32_t a);
uint8_t yc_transmit(Message *m);
void _pack_n_go();
void _take_off(Message *m);
}

void init() {
	Message m;
	
	DDRD |= (1 << PD4);
	
	m.id = 7;
	m.rtr = 0;
	m.length = 2;
	m.info = 0;
	m.data[0] = 0xF0;
	m.data[1] = 0x55;

	while(yc_transmit(&m) == 2)
		_delay_us(100);
}

void DM(SetStatus(uint8_t a, uint8_t b, uint16_t c, uint8_t d)) {
	// handle it
	yc_prepare(0x12345678);
	yc_send(Switch, SetStatus(12, 'c', 3, 1));
}

void DM(LightStatus(uint8_t a)) {
	uint16_t sp;
	
	if(a)
		PORTD |= (1 << PD4);
	else
		PORTD &= ~(1 << PD4);
	
	yc_prepare(0x12345678);
	yc_send(Switch, Status(a));
}

