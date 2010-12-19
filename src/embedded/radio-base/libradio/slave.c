#include <string.h>
#include <util/crc16.h>
#include "radio.h"
#include "../librfm12/rfm12.h"
#include "../librfm12/spi.h"
#include <avr/pgmspace.h>
#include <avr/interrupt.h>


extern RadioMessage msg_in, buf_out;
extern uint8_t buf_out_index;
extern volatile uint8_t msg_in_full;
extern uint8_t our_radio_id, target_id;
extern volatile radio_state_t radio_state;
volatile uint8_t rx_fc = 0, tx_fc = 0;

void _radio_rxc(int16_t data);
int16_t _radio_txc();


#define RFM12_select()          PORTB &= ~(1 << PB2)
#define RFM12_unselect()        PORTB |= (1 << PB2)

uint16_t _RFM12_trans(uint16_t wert)
{
	uint16_t timeout = 0;
	uint8_t sreg;
	CONVERTW val;
	val.w=wert;
	sreg = SREG;
	cli();
	RFM12_select();
	SPI_trans(val.b[1], timeout);
	SPI_trans(val.b[0], timeout);
	RFM12_unselect();
	SREG = sreg;
	return val.w;
}

void RFM12_LLC_receiveFinished() {}

void radio_init(uint8_t radio_id_node) { // we will only receive this ID
	RFM12_LLC_registerType(&_radio_rxc, &_radio_txc);
	our_radio_id = radio_id_node;

	// configure SPI
	DDRB |= _BV(DDB3) | _BV(DDB5) | _BV(DDB2); DDRB &= ~_BV(DDB4);
	SPI_master();		// Master mode
	SPI_disableINT();	// Disable SPI interrupt
	SPI_dordMSB();		// Data Order: MSB first
	SPI_cpolIdleLow();	// Clock Polarity: SCK low when idle
	SPI_cphaSampRise(); // Clock Phase: sample on rising SCK edge
	SPI_setFrequency(); // SPI frequency: ~ 3 MHz
	SPI_enable();
	RFM12_PHY_init();
	MCUCR &= ~(_BV(ISC01) | _BV(ISC00)); // RFM12_INT_init()
	GICR |= _BV(INT0); // RFM12_INT_on()
}

void protocol_dispatch(uint8_t radio_id, RadioMessage *msg) {
	// not for us?
	if(radio_id != our_radio_id)
		return;

	// verify CRC
	if(radio_crc(radio_id, msg) == msg->crc16) {
		rx_fc++;
		memcpy(&msg_in, msg, sizeof(msg_in));
		msg_in_full = 1;
	}
}

tstatus radio_retransmit(uint8_t radio_id, RadioMessage *msg) {
	if(radio_state != ST_IDLE)
		return PENDING;
	if(msg->info) // if we have already queued message and are done (idle)
		return SUCCESS;

	--tx_fc;
	return radio_transmit(radio_id, msg);
}

tstatus radio_transmit(uint8_t radio_id, RadioMessage *msg) {
	if(radio_state != ST_IDLE)
		return PENDING;
	if(msg->info) // if we have already queued message and are done (idle)
		return SUCCESS;

	// set frame counter, calculate CRC (changes original message)
	msg->fc = ++tx_fc;
	msg->crc16 = radio_crc(radio_id, msg);

	// queue message
	memcpy(&buf_out, msg, sizeof(buf_out));
	msg->info = 1;
	target_id = radio_id;

	radio_state = ST_TX;
	RFM12_LLC_sendFrame();

	return PENDING;
}

ISR(INT0_vect) {
	_RFM12_ISR();
}

