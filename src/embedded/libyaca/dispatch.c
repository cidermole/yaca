#include <avr/eeprom.h>
#include <util/delay.h>
#include <inttypes.h>
#include "dispatch.h"

/*

EEPROM: [2][CANid][CANid][1][CANid][3][CANid][CANid][CANid]

*/

uint32_t _global_canid;

void yc_prepare(uint32_t canid) {
	_global_canid = canid;
}

void yc_prepare_ee(uint8_t *pcanid) {
	eeprom_read_block(&_global_canid, pcanid, sizeof(_global_canid));
}

void _take_off(Message* m) {
	m->id = _global_canid;
	m->rtr = 0;
	m->length = 8;
	m->info = 0;

	if(yc_transmit(m) == PENDING) {
//		_delay_us(100);
		_delay_ms(1);
		while(yc_poll_transmit(m) == PENDING) // XXX wooot? wtf - why do we need delay?
//			_delay_us(100);
			_delay_ms(1);
	}
}


void yc_dispatch(Message* m, uint8_t* eep_idtable, void** flash_fps, uint8_t* flash_count) {
	uint8_t* pRom = eep_idtable;
	uint8_t* pFlash = (uint8_t*)flash_fps;
	uint8_t count, i, packstyle, flags;
	void (*fp)();
	uint32_t id;
	uint32_t m_id = m->id;

	packstyle = pgm_read_byte(pFlash++);
	flags = pgm_read_byte(pFlash++);
	fp = (void (*)())pgm_read_word((uint16_t*)pFlash);
	pFlash += 2;
	for(count = pgm_read_byte(flash_count); count > 0; count--) {
		// read all CANid's from EEPROM
		for(i = eeprom_read_byte(pRom++); i > 0; i--) {
			eeprom_read_block(&id, pRom, sizeof(id));
			pRom += sizeof(id);

			if(flags & (1 << 0)) { // RTR response function?
				if(!m->rtr)
					continue;
			} else if(m->rtr) {
				continue;
			}

			if(id == m_id) {
				unpack(m, packstyle);
				fp();
				return;
			}
		}
		packstyle = pgm_read_byte(pFlash++);
		flags = pgm_read_byte(pFlash++);
		fp = (void (*)())pgm_read_word((uint16_t*)pFlash);
		pFlash += 2;
	}
}

