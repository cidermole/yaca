#include "MainPower.h"
#include "RMainPower.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#ifndef F_CPU
#define F_CPU 2000000UL
#endif
#include <util/delay.h>
#include <yaca.h>
/*
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
*/
int main() {
	Message in;

	sei();

    yc_send(MainPower, HelloWorld(10));

	while(1) {
		yc_dispatch_auto();
	}
	return 0;
}

