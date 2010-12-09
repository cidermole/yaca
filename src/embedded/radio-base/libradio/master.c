#include <string.h>
#include <stdlib.h>
#include <util/crc16.h>
#include "radio.h"
#include "rijndael.h"
#include "../librfm12/rfm12.h"
#include "master.h"
#include <avr/pgmspace.h>
#include "/home/david/Info/yaca-aeskey.h"
// yaca-aeskey.h only contains the following line: const uint8_t _flash_aes_key[16] PROGMEM = {...};

#define PREFIX

extern RadioMessage msg_in, buf_out;
extern uint8_t buf_out_index, msg_in_full;
extern uint8_t our_radio_id, target_id;
extern radio_state_t radio_state;
extern uint8_t aes_key[AES_EXPKEY_SIZE];

void _radio_rxc(int16_t data);
int16_t _radio_txc();

// TODO: move to EEP
slot_assign_t slot_assignments[] = {
	// radio_id, slot
	{2, 0}
};

slot_t slots[5]; // size: 51 bytes * elements


void radio_init(uint8_t radio_id_node) { // we will only receive this ID
	uint8_t i, j;

	aes_key_expand(aes_key, _flash_aes_key, AES_KEY_FLASH);

	RFM12_LLC_registerType(&_radio_rxc, &_radio_txc);
	our_radio_id = radio_id_node;

	memset(slots, 0, sizeof(slots)); // set states and fc to 0
	for(i = 0; i < sizeof(slots) / sizeof(slot_t); i++) {
		slots[i].rx_fc = 0;
		slots[i].tx_fc = 0;

		for(j = 0; j < sizeof(slots[i].tx_state); j++)
			slots[i].tx_state[j] = (uint8_t) random(); // non-standard avr-libc 32-bit random number generator

		memcpy(slots[i].rx_state, slots[i].tx_state, sizeof(slots[i].rx_state));
	}

	// configure SPI
	DDRB |=                         _BV(DDB1);
	RFM12_PHY_init();
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

	if(retr == RETRY) {
		memcpy(slot->tx_state, slot->tx_state_old, sizeof(slot->tx_state));
	} else {
		if(++slot->tx_fc == 0xFF) // avoid special code FF
			slot->tx_fc = 0;
		msg.fc = slot->tx_fc;
		memcpy(slot->tx_state_old, slot->tx_state, sizeof(slot->tx_state_old));
	}
	_master_radio_transmit(radio_id, &msg, slot->tx_state);
}

void protocol_dispatch(uint8_t radio_id, RadioMessage *msg) {
	slot_t *slot;
	RadioMessage plain;
	uint8_t state[16];

	if((slot = find_slot(radio_id)) == NULL) { // if no slot found, we recv'd incorrect data
		return;
	}

	if(msg->fc == slot->rx_fc) { // retransmission received?
		// We just assume that the frame is correct.
		// An attacker could hereby make us send the same ACK
		// which we've already transmitted (no big deal).
		_send_ack(radio_id, slot, RETRY);
	} else if(msg->fc == slot->rx_fc + 1 || (slot->rx_fc == 0xFE && msg->fc == 0)) {
		memcpy(state, slot->rx_state, sizeof(state)); // backup state
		aes_decrypt(aes_key, &((uint8_t *) msg)[2], &((uint8_t *) &plain)[2], slot->rx_state);
		plain.fc = msg->fc;
		// verify CRC
		if(radio_crc(radio_id, &plain) == plain.crc16) {
			if(++slot->rx_fc == 0xFF) // avoid special code FF
				slot->rx_fc = 0;
			if(!msg_in_full) {
				memcpy(&msg_in, msg, sizeof(RadioMessage));
				msg_in_full = 1;
			}
			_send_ack(radio_id, slot, NORMAL);
		} else {
			int i;
			memcpy(slot->rx_state, state, sizeof(state)); // restore state
			for(i = 0; i < sizeof(RadioMessage); i++) {
			}
			for(i = 0; i < sizeof(RadioMessage); i++) {
			}
		}
	} else if(msg->fc == 0xFF) { // resync (AES state request)
		// TODO: we should only enable resync on user action
		slot->rx_fc = 0;
		slot->tx_fc = 0;
		memcpy(&((uint8_t *) &buf_out)[1], slot->tx_state, sizeof(slot->tx_state));
		memcpy(slot->rx_state, slot->tx_state, sizeof(slot->tx_state));
		target_id = radio_id | 0x80; // MSB indicates AES state transmission

		radio_state = ST_TX;
		RFM12_LLC_sendFrame();
	} else {
		// possible attack
	}
}

tstatus radio_transmit(uint8_t radio_id, RadioMessage *msg) {
	slot_t *slot;

	if((slot = find_slot(radio_id)) == NULL) // if no slot found, we got incorrect data
		return FAILURE;

	return _master_radio_transmit(radio_id, msg, slot->tx_state);
}

tstatus _master_radio_transmit(uint8_t radio_id, RadioMessage *msg, uint8_t *tx_state) {

	if(radio_state != ST_IDLE)
		return PENDING;
	if(msg->info) // if we have already queued message and are done (idle)
		return SUCCESS;

	// calculate CRC (changes original message)
	msg->crc16 = radio_crc(radio_id, msg);

	// encrypt and queue message
	aes_encrypt(aes_key, &((uint8_t *) msg)[2], &((uint8_t *) &buf_out)[2], tx_state);
	buf_out.fc = msg->fc;
	msg->info = 1;
	target_id = radio_id;

	radio_state = ST_TX;
	RFM12_LLC_sendFrame();

	return PENDING;
}

