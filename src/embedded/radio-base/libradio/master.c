#include <string.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include "radio.h"
#include "../librfm12/rfm12.h"
#include "../librfm12/spi.h"
#include "master.h"
#include <avr/pgmspace.h>

#define PREFIX

extern RadioMessage msg_in, buf_out;
extern uint8_t buf_out_index, msg_in_full;
extern uint8_t our_radio_id, target_id;
extern volatile radio_state_t radio_state;

void _radio_rxc(int16_t data);
int16_t _radio_txc();

// TODO: move to EEP
slot_assign_t slot_assignments[] = {
	// radio_id, slot
	{2, 200}
};

typedef struct {
	uint8_t radio_id;
	slot_t *slot;
	retr_e retr;
} deferred_ack_t;

slot_t slots[5]; // size: 51 bytes * elements


#define RFM12_select()          PORTB &= ~(1 << PB1)
#define RFM12_unselect()        PORTB |= (1 << PB1)

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

void radio_init(uint8_t radio_id_node) { // we will only receive this ID
	RFM12_LLC_registerType(&_radio_rxc, &_radio_txc);
	our_radio_id = radio_id_node;

	memset(slots, 0, sizeof(slots)); // set fc to 0

	// configure SPI
	DDRB |=                         _BV(DDB1);
	RFM12_PHY_init();
	MCUCR &= ~(_BV(ISC11) | _BV(ISC10)); // RFM12_INT_init()
	GICR |= _BV(INT1); // RFM12_INT_on()
}

slot_t *find_slot(uint8_t radio_id) {
	uint8_t i;

	for(i = 0; i < sizeof(slot_assignments) / sizeof(slot_assign_t); i++) {
		if(slot_assignments[i].radio_id == radio_id)
			return &slots[i];
	}
	return NULL;
}

extern uint16_t ms_timer(); // implement this!

void _send_ack(uint8_t radio_id, slot_t *slot, retr_e retr) {
	RadioMessage msg;
	int32_t target_time, cur_time = ms_timer();
	slot_assign_t *sa = &slot_assignments[slot - slots];
	int16_t time_feedback;

	target_time = ((int32_t) sa->slot) * SLOT_LENGTH - cur_time;
	if(target_time < -30000)
		target_time += 60000;
	time_feedback = (int16_t) target_time;

	memset(&msg, 0, sizeof(msg));
	msg.flags.ack = 1;
	msg.length = 2;
	msg.data[0] = (uint8_t) time_feedback;
	msg.data[1] = (uint8_t) (time_feedback >> 8);

	if(retr != RETRY) {
		slot->tx_fc++;
		msg.fc = slot->tx_fc;
	}
	_master_radio_transmit(radio_id, &msg);
}

extern void debug_tx(volatile uint8_t *p);

volatile deferred_ack_t deferred_ack = {0, 0, 0};

void _defer_ack(uint8_t radio_id, slot_t *slot, retr_e retr) {
	deferred_ack.radio_id = radio_id;
	deferred_ack.slot = slot;
	deferred_ack.retr = retr;
}

void RFM12_LLC_receiveFinished() {
	// if there is a deferred ack, execute it
	if(deferred_ack.slot != NULL)
		_send_ack(deferred_ack.radio_id, deferred_ack.slot, deferred_ack.retr);
	deferred_ack.slot = NULL;
}

void protocol_dispatch(uint8_t radio_id, RadioMessage *msg) {
	slot_t *slot;

	if((slot = find_slot(radio_id)) == NULL) { // if no slot found, we recv'd incorrect data
		return;
	}

	if(msg->fc == slot->rx_fc) { // retransmission received?
		// We just assume that the frame is correct.
		// An attacker could hereby make us send the same ACK
		// which we've already transmitted (no big deal).
		_defer_ack(radio_id, slot, RETRY);
	} else if(msg->fc == slot->rx_fc + 1) {
		if(radio_crc(radio_id, msg) == msg->crc16) {
			slot->rx_fc++;
			if(!msg_in_full) {
				memcpy(&msg_in, msg, sizeof(RadioMessage));
				msg_in_full = 1;
			}
			_defer_ack(radio_id, slot, NORMAL);
		}
	}
}

tstatus radio_transmit(uint8_t radio_id, RadioMessage *msg) {
	slot_t *slot;

	if((slot = find_slot(radio_id)) == NULL) // if no slot found, we got incorrect data
		return FAILURE;

	return _master_radio_transmit(radio_id, msg);
}

tstatus _master_radio_transmit(uint8_t radio_id, RadioMessage *msg) {

	if(radio_state != ST_IDLE)
		return PENDING;
	if(msg->info) // if we have already queued message and are done (idle)
		return SUCCESS;

	// calculate CRC (changes original message)
	msg->crc16 = radio_crc(radio_id, msg);

	// queue message
	buf_out = *msg;
	msg->info = 1;
	target_id = radio_id;

	radio_state = ST_TX;
	RFM12_LLC_sendFrame();

	return PENDING;
}

ISR(INT1_vect) {
	_RFM12_ISR();
}

