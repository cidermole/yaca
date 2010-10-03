#ifndef OW_CLIENT_H
#define OW_CLIENT_H

#include <stdint.h>
#include <avr/io.h>

#define OWC_DDR  DDRB
#define OWC_PORT PORTB
#define OWC_PIN  PINB
#define OWC_BIT  PB1

void owc_init();
void owc_wait_reset();
uint8_t owc_read();
void owc_write(uint8_t byte);

#ifndef set_bit
#define set_bit(var, bit) ((var) |= (1 << (bit)))
#define clear_bit(var, bit) ((var) &= ~(1 << (bit)))
#define toggle_bit(var, bit) ((var) ^= (1 << (bit)))
#endif

#endif /* OW_CLIENT_H */

