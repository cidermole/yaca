#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include <yaca-bl.h>
#include <inttypes.h>
#include <avr/io.h>


#include "eeprom.h"


// time to wait after power-up before booting app in ms
#define BLD_TIMEOUT 3000

// page count for Atmega8 = 128
#define BLD_PAGE_COUNT ((FLASHEND + 1) / SPM_PAGESIZE)

// app page count for Atmega8 while using 2K (the largest setting) for bootloader section
#define BLD_APP_PAGE_COUNT 96

// page size in bytes, for Atmega8 = 64
#define BLD_PAGE_SIZE SPM_PAGESIZE

// time to wait for EEPROM "warm-up" after powerup in ms
#define EEPROM_WARMUP_DELAY 1000


#define BLD_ERR_MSG_OVERFLOW 9


#define BIOSRAM __attribute__((section (".biosram")))
#define bytewise(var, b) (((uint8_t*)&(var))[b])

void bootApp();
void flashPage(uint8_t page, uint8_t* buffer);
uint8_t receiveValidateTempidMsg(Message* msg);

#endif /* _BOOTLOADER_H_ */

