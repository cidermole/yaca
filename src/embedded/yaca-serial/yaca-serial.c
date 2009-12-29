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

#define CANID_TIME 401
#define TIME_FLAGS_DST (1 << 0) // daylight saving time
#define TIME_FLAGS_BAK (1 << 1) // backup time source

#define LINUX_PORT PORTD
#define LINUX_DDR  DDRD
#define LINUX_BIT  (1 << PD5)

#define bytewise(var, b) (((uint8_t*)&(var))[b])

static uint8_t state = 0;
volatile uint8_t sub_count = 0, hour = 0, min = 0, sec = 0, day = 1, month = 1, year = 0, ntp = 1, dst = 0, tr_time = 0;

void delay_ms(uint16_t t) {
	uint16_t i;
	for(i = 0; i < t; i++)
		_delay_ms(1);
}

void uart_put_message(Message *m) {
	uint8_t i, j;
	
	uart_putc(REPLY_DATA);
	for(i = 0; i < (sizeof(Message) - 8 + m->length); i++) { // rationalize by omitting unused message bytes
		j = bytewise(*m, i);
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
	static Message msg_out;
	static uint8_t msg_index = 0;
	static uint8_t mask = 0;

	switch(state) {
	case 0:
		state = do_cmd(c);
		break;

	case 1: // Command: Send CAN frame
		if(mask && c == 0x00) {
			if(msg_out.id == CANID_TIME && msg_out.rtr == 0) {
				ntp = 1;
				sub_count = 0;
				hour = msg_out.data[0];
				min = msg_out.data[1];
				sec = msg_out.data[2];
				year = msg_out.data[3];
				month = msg_out.data[4];
				day = msg_out.data[5];
				dst = msg_out.data[6] & 0x01;
			}
			
			msg_index = 0;
			if(yc_transmit(&msg_out) == PENDING) {
				state = 2;
			} else {
				uart_putc(REPLY_TRSUC);
				state = 0;
			}
			mask = 0;
		} else if(!mask && c == 0x55) {
			mask = 1;
		} else {
			if(mask && c == 0x01) {
				mask = 0;
				c = 0x55;
			}
		
			if(msg_index >= sizeof(Message)) // XXX why do we need this? (dies after 256 msgs from uart if not here)
				msg_index = 0;
			bytewise(msg_out, msg_index) = c;
			if(++msg_index >= sizeof(Message)) // don't let msg_index overflow
				msg_index = 0;
		}
		break;

	case 2:
		_delay_us(100);
		if(yc_poll_transmit(&msg_out) != PENDING) {
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
	uint8_t wb_reported = 0, rb_reported = 0;
	Message msg;

	uart_init((uint16_t) (F_CPU / (16.0 * BAUDRATE) - 1));
	
	delay_ms(1000); // Wait for EEPROM to warm up (?)
	
	yc_init();
	
	TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC, prescaler 64
	OCR1A = 31250; // 64 * 3.125 = 200.000 (100 ms ticks)
	
	wdt_enable(WDTO_2S);
	sei();

	while(1) {
		if(fifo2_read.count && state != 2 && state != 3) {
			data = uart_getc_nowait();
			do_uart((uint8_t)data);
		}
		if(state == 2) { // if we are waiting for yc_transmit()...
			do_uart(0); // the state machine needs to run to be able to poll for completion
		}
		
		if(yc_poll_receive()) {
			yc_receive(&msg);
			uart_put_message(&msg);
		}
		
		if(state == 3) {
			_delay_us(100);
			if(yc_poll_transmit(&msg) != PENDING) {
				state = 0;
			}
		}
		
		if(tr_time && state != 2) {
			msg.info = 0;
			msg.id = CANID_TIME;
			msg.length = 7;
			msg.data[0] = hour;
			msg.data[1] = min;
			msg.data[2] = sec;
			msg.data[3] = year;
			msg.data[4] = month;
			msg.data[5] = day;
			msg.data[6] = dst | TIME_FLAGS_BAK;
			
			if(yc_transmit(&msg) == PENDING) {
				state = 3;
			}
			
			// debug: send time back to tty
			uart_put_message(&msg);
			
			tr_time = 0;
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

void advance_time() {
	uint8_t days_in_month;
	
	sec++;
	if(sec >= 60) {
		sec = 0;
		min++;
	} else {
		return;
	}
	
	if(min >= 60) {
		min = 0;
		hour++;
	} else {
		return;
	}
	
	if(hour >= 24) {
		hour = 0;
		day++;
	} else {
		return;
	}
	
	switch(month) {
		case 1: days_in_month = 31; break;
		case 2:
			// (2000 + year) % 4 == 0    ->   year % 4 == 0
			// (2000 + year) % 100 == 0  ->   year % 100 == 0
			// XXX: 400-year rule not implemented - doesn't make sense on 8 bit
			
			if((year % 4 == 0) && (year % 100 != 0))
				days_in_month = 29;
			else
				days_in_month = 28;
			break;
		case 3: days_in_month = 31; break;
		case 4: days_in_month = 30; break;
		case 5: days_in_month = 31; break;
		case 6: days_in_month = 30; break;
		case 7: days_in_month = 31; break;
		case 8: days_in_month = 31; break;
		case 9: days_in_month = 30; break;
		case 10: days_in_month = 31; break;
		case 11: days_in_month = 30; break;
		case 12: days_in_month = 31; break;
		default: days_in_month = 30;
	}
	
	if(day > days_in_month) {
		day = 1;
		month++;
	} else {
		return;
	}
	
	if(month > 12) {
		month = 1;
		year++;
	}
}

ISR(TIMER1_COMPA_vect) {
	sub_count++;
	if(ntp == 1 && sub_count >= 12) {
		ntp = 0;
	} else if(ntp || sub_count < 10) {
		return;
	}
	sub_count = 0;
	advance_time();
	tr_time = 1;
}

