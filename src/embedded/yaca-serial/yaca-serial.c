#include <util/delay.h>
#include <inttypes.h>
#include <avr/wdt.h>
#include <yaca-bl.h>

#include "uart.h"
#include "calendar.h"

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

#define CLOCK_CORR 5454

static uint8_t state = 0;
volatile uint8_t sub_count = 0, hour = 0, min = 0, sec = 0, day = 1, month = 1, ntp = 1, dst = 0, tr_time = 0;
volatile uint16_t year = 0, corr_fac = 0;

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
				year = msg_out.data[4];
				year += ((uint16_t) msg_out.data[3]) << 8;
				month = msg_out.data[5];
				day = msg_out.data[6];
				dst = msg_out.data[7] & 0x01;
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
	OCR1A = 3125; // 64 * 3.125 = 200.000 (100 ms ticks)
	TIMSK = (1 << OCIE1A); // enable output-compare interrupt
	
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
			msg.rtr = 0;
			msg.id = CANID_TIME;
			msg.length = 8;
			msg.data[0] = hour;
			msg.data[1] = min;
			msg.data[2] = sec;
			msg.data[4] = year;
			msg.data[3] = year >> 8;
			msg.data[5] = month;
			msg.data[6] = day;
			msg.data[7] = dst | TIME_FLAGS_BAK;
			
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
		
		// CEST starts on the last Sunday of March at 02:00 CET
		if(month == 3 && hour == 2 && (day + 7) > 31 && day_of_week(year, month, day) == 0) {
			dst = 1;
			hour = 3;
		}
		
		// CEST ends on the last Sunday of October at 03:00 CEST
		if(dst == 1 && month == 10 && hour == 3 && (day + 7) > 31 && day_of_week(year, month, day) == 0) {
			dst = 0;
			hour = 2;
		}
	} else {
		return;
	}
	
	if(hour >= 24) {
		hour = 0;
		day++;
	} else {
		return;
	}
	
	if(day > days_in_month(month, year)) {
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
	if(++corr_fac == CLOCK_CORR) {
		corr_fac = 0;
		sub_count++;
	}
	if(ntp == 1 && sub_count >= 32) { // if 3 updates are missing, switch to local time sync
		advance_time();
		advance_time();
		sub_count -= 20;
		ntp = 0;
	} else if(ntp || sub_count < 10) {
		return;
	}
	sub_count -= 10;
	advance_time();
	
	// don't transmit time if we were cold-started...
	if(year != 0)
		tr_time = 1;
}

