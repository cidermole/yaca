#include <stdio.h>
#include <string.h>
#if defined(__AVR__)
#include <util/crc16.h>
#endif
#include "radio.h"
#include "rijndael.h"
#include "../librfm12/rfm12.h"
#include "master.h"

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
	{1, 0}
};

slot_t slots[5]; // size: 51 bytes * elements


void radio_init(uint8_t radio_id_node) { // we will only receive this ID
	uint8_t _tx_key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}; // TODO should be in EEPROM
	// TODO for base station: random from SRAM startup content (with CRC? with xrandom() from aestable.c?)
	// TODO load states for every slot

	aes_key_expand(aes_key, _tx_key, AES_KEY_SRAM);

	RFM12_LLC_registerType(&_radio_rxc, &_radio_txc);
	our_radio_id = radio_id_node;

	memset(slots, 0, sizeof(slots));
}

slot_t *find_slot(uint8_t radio_id) {
	uint8_t i;

	for(i = 0; i < sizeof(slot_assignments) / sizeof(slot_assign_t); i++) {
		if(slot_assignments[i].radio_id == radio_id)
			return &slots[i];
	}
	return NULL;
}

void _send_ack(uint8_t radio_id, slot_t *slot, retr_e retr) {
	RadioMessage msg;
	memset(&msg, 0, sizeof(msg));

	if(retr == RETRY) {
		memcpy(slot->tx_state, slot->tx_state_old, sizeof(slot->tx_state));
	} else {
		msg.fc = ++slot->tx_fc;
		memcpy(slot->tx_state_old, slot->tx_state, sizeof(slot->tx_state_old));
	}
	_master_radio_transmit(radio_id, &msg, slot->tx_state);
}

void protocol_dispatch(uint8_t radio_id, RadioMessage *msg) {
	slot_t *slot;
	slot_assign_t *sa;
	RadioMessage plain;

	if((slot = find_slot(radio_id)) == NULL) // if no slot found, we recv'd incorrect data
		return;
	sa = &slot_assignments[slot - slots];

	if(msg->fc == slot->rx_fc) { // retransmission received?
		// We just assume that the frame is correct.
		// An attacker could hereby make us send the same ACK
		// which we've already transmitted (no big deal).
		_send_ack(radio_id, slot, RETRY);
	} else if(msg->fc == slot->rx_fc + 1) {
		aes_decrypt(aes_key, &((uint8_t *) &msg)[1], &((uint8_t *) &plain)[1], slot->rx_state);
		// verify CRC
		if(radio_crc(radio_id, msg) == msg->crc16) {
			fprintf(stderr, "protocol_dispatch(): CRC correct\n");
			slot->rx_fc++;
			_send_ack(radio_id, slot, NORMAL);
		}
	} else {
		// possible attack
	}
}

void _radio_rxc(int16_t data) {
	static uint8_t id_in, buf_in_index = 0;
	static RadioMessage buf_in;

	if(data == RFM12_L3_EOD) {
		fprintf(stderr, PREFIX " EOF. got %d bytes\n", (int)buf_in_index);
		radio_state = ST_IDLE;
		protocol_dispatch(id_in, &buf_in);
		buf_in_index = 0;
	} else {
		radio_state = ST_RX;
		if(buf_in_index == 0) {
			id_in = data;
			buf_in_index++;
		} else if(buf_in_index < sizeof(RadioMessage)) {
			((uint8_t *) &buf_in)[buf_in_index++] = data;
		}
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
	aes_encrypt(aes_key, &((uint8_t *) msg)[1], &((uint8_t *) &buf_out)[1], tx_state);
	msg->info = 1;
	target_id = radio_id;

	radio_state = ST_TX;
	RFM12_LLC_sendFrame();

	return PENDING;
}

