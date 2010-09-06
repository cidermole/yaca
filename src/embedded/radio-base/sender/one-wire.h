#ifndef _ONE_WIRE_H_
#define _ONE_WIRE_H_

#include <inttypes.h>


#define OW_DDR  DDRB
#define OW_PORT PORTB
#define OW_PIN  PINB
#define OW_BIT  PB0


#define OW_SKIP_ROM        0xCC
#define OW_CONVERT_T       0x44
#define OW_READ_SCRATCHPAD 0xBE

void ow_write(uint8_t b);
uint8_t ow_read();
void ow_pull();
void ow_release();
uint8_t ow_check();

#ifndef set_bit
#define set_bit(var, bit) ((var) |= (1 << (bit)))
#define clear_bit(var, bit) ((var) &= ~(1 << (bit)))
#define toggle_bit(var, bit) ((var) ^= (1 << (bit)))
#endif

#endif /* _ONE_WIRE_H_ */
