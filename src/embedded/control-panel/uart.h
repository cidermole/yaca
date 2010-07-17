#ifndef __UART_H
#define __UART_H

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "fifo.h"

#define BAUDRATE 115200

#define WRITE_BUFFER 200
#define READ_BUFFER  200

extern fifo_t fifo2_read;
extern fifo_t fifo2_write;

uint8_t wb_is_full();
uint8_t rb_is_full();

void uart_init(uint16_t ubrr);
int uart_putc(const uint8_t c);
int uart_puts(const char* p);
int uart_puts_P(const prog_char* p);
void uart_put_hex(uint8_t number);

#define uart_getc_wait() fifo2_get_wait(&fifo2_read)
#define uart_getc_nowait() fifo2_get_nowait(&fifo2_read)


#endif /* __UART_H */
