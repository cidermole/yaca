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
	{2, 0}
};

slot_t slots[5]; // size: 51 bytes * elements


void radio_init(uint8_t radio_id_node) { // we will only receive this ID
	uint8_t _tx_key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}; // TODO should be in EEPROM
	// TODO for base station: random from SRAM startup content (with CRC? with xrandom() from aestable.c?)
	// TODO load states for every slot

	aes_key_expand(aes_key, _tx_key, AES_KEY_SRAM);

	RFM12_LLC_registerType(&_radio_rxc, &_radio_txc);
	our_radio_id = radio_id_node;

	memset(slots, 0, sizeof(slots)); // set states and fc to 0
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
	int16_t time_feedback = 0; // TODO read timer

	memset(&msg, 0, sizeof(msg));
	msg.flags.ack = 1;
	msg.length = 2;
	msg.data[0] = (uint8_t) time_feedback;
	msg.data[1] = (uint8_t) (time_feedback >> 8);

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
	uint8_t state[16];

	if((slot = find_slot(radio_id)) == NULL) { // if no slot found, we recv'd incorrect data
		fprintf(stderr, PREFIX "protocol_dispatch(): no slot found\n");
		return;
	}
	sa = &slot_assignments[slot - slots];

	if(msg->fc == slot->rx_fc) { // retransmission received?
		// We just assume that the frame is correct.
		// An attacker could hereby make us send the same ACK
		// which we've already transmitted (no big deal).
		_send_ack(radio_id, slot, RETRY);
	} else if(msg->fc == slot->rx_fc + 1) {
		memcpy(state, slot->rx_state, sizeof(state)); // backup state
		aes_decrypt(aes_key, &((uint8_t *) msg)[2], &((uint8_t *) &plain)[2], slot->rx_state);
		plain.fc = msg->fc;
		// verify CRC
		if(radio_crc(radio_id, &plain) == plain.crc16) {
			fprintf(stderr, PREFIX "protocol_dispatch(): CRC correct\n");
			slot->rx_fc++;
			if(!msg_in_full) {
				memcpy(&msg_in, msg, sizeof(RadioMessage));
				msg_in_full = 1;
			}
			_send_ack(radio_id, slot, NORMAL);
		} else {
			int i;
			memcpy(slot->rx_state, state, sizeof(state)); // restore state
			fprintf(stderr, PREFIX "protocol_dispatch(): CRC error. radio_id: %d. CRC should be %04X\n", radio_id, (int) radio_crc(radio_id, &plain));
			for(i = 0; i < sizeof(RadioMessage); i++) {
				fprintf(stderr, PREFIX " %02X", (int)(((uint8_t *) msg)[i]));
			}
			fprintf(stderr, "\n");
			for(i = 0; i < sizeof(RadioMessage); i++) {
				fprintf(stderr, PREFIX " %02X", (int)(((uint8_t *) &plain)[i]));
			}
			fprintf(stderr, "\n");
		}
	} else {
		// possible attack
		fprintf(stderr, PREFIX "protocol_dispatch(): possible attack: rx_fc = %d, msg->fc = %d\n", slot->rx_fc, msg->fc);
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

