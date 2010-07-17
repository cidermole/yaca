#include "uart.h"
#include "fifo.h"


uint8_t buf2_read[READ_BUFFER];
uint8_t buf2_write[WRITE_BUFFER];

fifo_t fifo2_read;
fifo_t fifo2_write;

void uart_init(uint16_t ubrr) {
	UBRRH = (uint8_t) (ubrr>>8);
	UBRRL = (uint8_t) (ubrr);

	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE) | (1 << TXCIE);
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
//	UCSRA = (1 << U2X);

	// Flush Receive-Buffer
	do {
		UDR;
	} while (UCSRA & (1 << RXC));

	fifo2_init(&fifo2_read, buf2_read, READ_BUFFER);
	fifo2_init(&fifo2_write, buf2_write, WRITE_BUFFER);
}

int uart_putc(const uint8_t c) {
	if(0 == fifo2_write.count && (UCSRA & (1 << UDRE)))
		UDR = c;
	else
		return fifo2_put(&fifo2_write, c);

	return 1;
}

int uart_puts_P(const prog_char* p) {
	char c = (char) pgm_read_byte(p++);
	int r = 1;

	while('\0' != c)
		if(!(r = uart_putc(c)))
			c = '\0';
		else
			c = (char) pgm_read_byte(p++);

	return r;
}

int uart_puts(const char* p) {
	char c = *p;
	int r = 1;

	while('\0' != c)
		if(!(r = uart_putc(*p)))
			c = '\0';
		else
			c = *(p++);

	return r;
}

ISR(USART_RXC_vect) {
	uint8_t c = UDR;
	fifo2_put(&fifo2_read, c);
}

ISR(USART_TXC_vect) {
	int temp;

	if(-1 != (temp = fifo2_get_nowait(&fifo2_write))) {
		UDR = (uint8_t) temp;
	}
}
