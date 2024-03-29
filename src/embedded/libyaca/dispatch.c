#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

#include "dispatch.h"
#include "../bootloader/eeprom.h"
#include "../bootloader/msgdefs.h"

void __bld_reset(void);
void __attribute__ ((weak)) enter_bootloader_hook(void);

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

void yc_bld_reset() {
	cli();
	MCUCSR &= ~(1 << PORF); // Clear Power-On Reset Flag: tell bootloader that we came from app (no need for another TID_BLD_ENTER)
	__bld_reset();
}

void _take_off(Message* m) {
	m->id = _global_canid;
	m->rtr = 0;
	m->length = 8;
	m->info = 0;

	if(yc_transmit(m) == PENDING) {
		_delay_us(100);
		while(yc_poll_transmit(m) == PENDING) // XXX wooot? wtf - why do we need delay?
			_delay_us(100);
	}
}


void yc_dispatch(Message* m, uint8_t* eep_idtable, void** flash_fps, uint8_t* flash_count) {
	uint8_t* pRom = eep_idtable;
	uint8_t* pFlash = (uint8_t*)flash_fps;
	uint8_t count, i, packstyle, flags;
	void (*fp)();
	uint32_t id, tid;
	uint32_t m_id = m->id;

	eeprom_read_block(&tid, EE_TEMPID, sizeof(tid));
	if(m_id == tid) {
		if(m->data[0] == TID_BLD_ENTER) {
			if(enter_bootloader_hook)
				enter_bootloader_hook();
			else
				yc_bld_reset();
		}
		return;
	}

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
				// XXX: breaks the register contents we have unpacked to (sets up address for icall), fix in asm below
				//fp();

				// 'Q' constraint (Y or Z ptr with displacement) needs avr-gcc >= 4.2.x
				asm volatile(
					"ldd  r30, %Q0" "\n\t"
					"ldd  r31, %Q0+1" "\n\t"
					"icall"
						:
						: "Q" (fp)
						: "r30", "r31"
				);
				// XXX: watch out here, the clobber list (Z pointer) is incomplete, the function call might have invalidated other registers
				// it is OK to return immediately though
				return;
			}
		}
		packstyle = pgm_read_byte(pFlash++);
		flags = pgm_read_byte(pFlash++);
		fp = (void (*)())pgm_read_word((uint16_t*)pFlash);
		pFlash += 2;
	}
}

