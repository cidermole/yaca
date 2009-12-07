#include <util/delay.h>
#include <inttypes.h>
#include <avr/wdt.h>
#include <yaca-bl.h>

#include "uart.h"

/*

yaca-gw protocol: line ends are tr'ed as 0x55 0x00
                  0x55 is transmitted as 0x55 0x01
                  ping reply is tr'ed as 0x55 0x02
                  
                  write buffer full:     0x55 0x03      OK: 0x55 0x13
                  read buffer full:      0x55 0x04      OK: 0x55 0x14

program start (connecting); usual ping procedure:
* send CMD_PING
* wait for 0x55 0x02 with timeout (except when receiving other bytes...)

e.g. 1 ping / second

*/

#define CMD_START   0x00 // Start Linux
#define CMD_RESTART 0x01 // Restart Linux
#define CMD_PING    0x02 // Ping hardware to check if it is alive
#define CMD_SEND    0x03 // Send CAN frame, frame format is Message in <yaca-bl.h>

#define REPLY_SUCCESS 0x00
#define REPLY_TRSUC   0x02
#define REPLY_DATA    0x01

#define LINUX_PORT PORTD
#define LINUX_DDR  DDRD
#define LINUX_BIT  (1 << PD5)

#define bytewise(var, b) (((uint8_t*)&(var))[b])

static uint8_t state = 0;

void delay_ms(uint16_t t) {
	uint16_t i;
	for(i = 0; i < t; i++)
		_delay_ms(1);
}

uint8_t do_cmd(uint8_t cmd) {
	switch(cmd) {
	case CMD_RESTART: // Restart Linux
		LINUX_PORT &= ~LINUX_BIT;
		LINUX_DDR |= LINUX_BIT;
		wdt_disable();
		delay_ms(7000);
		LINUX_DDR &= ~LINUX_BIT;
		delay_ms(1000);
		wdt_enable(WDTO_2S); // NO break

	case CMD_START: // Start Linux
		LINUX_PORT &= ~LINUX_BIT;
		LINUX_DDR |= LINUX_BIT;
		delay_ms(500);
		LINUX_DDR &= ~LINUX_BIT;
		uart_putc(REPLY_SUCCESS);
		break;

	case CMD_PING: // Ping
		uart_putc(0x55);
		uart_putc(0x02);
		uart_putc(REPLY_SUCCESS);
		break;

	case CMD_SEND: // Send CAN frame
		return 1;
//		break;

	default:
		break;
	}
	return 0;
}

void do_uart(uint8_t c) {
//	static uint8_t state = 0;
	static Message msg_out;
	static uint8_t msg_index = 0;
	static uint8_t mask = 0;

	switch(state) {
	case 0:
		state = do_cmd(c);
		break;

	case 1: // Command: Send CAN frame
		if(mask) {
			if(c == 0x00) { // end of UART message
				msg_index = 0;
				if(yc_transmit(&msg_out) == PENDING) {
					state = 2;
				} else {
					uart_putc(REPLY_TRSUC);
					state = 0;
				}
			} else if(c == 0x01) {
				c = 0x55;
			}
			mask = 0;
		} else if(c == 0x55) {
			mask = 1;
			break;
		}

		if(msg_index >= sizeof(Message)) // XXX why do we need this? (dies after 256 msgs from uart if not here)
			msg_index = 0;
		bytewise(msg_out, msg_index) = c;
		if(++msg_index >= sizeof(Message)) // don't let msg_index overflow
			msg_index = 0;
		break;

	case 2:
		_delay_us(100);
		if(yc_transmit(&msg_out) != PENDING) {
			uart_putc(REPLY_TRSUC);
			state = 0;
			msg_index = 0;
		}
		break;

	default:
		break;
	}
}

int main() {
	int data;
	uint8_t i, j, wb_reported = 0, rb_reported = 0;
	Message msg;

	uart_init((uint16_t) (F_CPU / (16.0 * BAUDRATE) - 1));
	
	delay_ms(1000); // Wait for EEPROM to warm up (?)
	
	yc_init();

	wdt_enable(WDTO_2S);
	sei();

	while(1) {
//		if((data = uart_getc_nowait()) != -1) {
		if(fifo2_read.count && state != 2) {
			data = uart_getc_nowait();
			do_uart((uint8_t)data);
		}
		if(state == 2) {
			do_uart(0);
		}

		if(yc_poll_receive()) {
			yc_receive(&msg);

			uart_putc(REPLY_DATA);
			for(i = 0; i < (sizeof(Message) - 8 + msg.length); i++) { // rationalize by omitting unused message bytes
				j = bytewise(msg, i);
				if(j == 0x55) {
					uart_putc(0x55);
					uart_putc(0x01);
				} else {
					uart_putc(j);
				}
			}
			uart_putc(0x55);
			uart_putc(0x00);
		}
		
		if(wb_is_full()) {
			if(!wb_reported) {
				uart_putc(0x55);
				uart_putc(0x03);
				wb_reported = 1;
			}
		} else {
			if(wb_reported) {
				uart_putc(0x55);
				uart_putc(0x13);
			}
			wb_reported = 0;
		}
		
		if(rb_is_full()) {
			if(!rb_reported) {
				uart_putc(0x55);
				uart_putc(0x04);
				rb_reported = 1;
			}
		} else {
			if(rb_reported) {
				uart_putc(0x55);
				uart_putc(0x14);
			}
			rb_reported = 0;
		}

		wdt_reset();
	}
	return 0;
}

