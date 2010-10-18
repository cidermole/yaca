#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../librfm12/rfm12.h"
#include "radio.h"
#include "rijndael.h"

RFM12_L3_rxCallback *rxC = 0;
RFM12_L3_txCallback *txC = 0;

uint8_t buffer[128], ibuf = 0;

void RFM12_LLC_registerType(RFM12_L3_rxCallback *rxCallback, RFM12_L3_txCallback *txCallback) {
	printf("RFM12_LLC_registerType()\n");
	rxC = rxCallback;
	txC = txCallback;
}

uint8_t RFM12_LLC_sendFrame() {
	int res, i;

	res = txC();
	ibuf = 0;
	buffer[ibuf++] = res;

	while(res != RFM12_L3_EOD) {
		fprintf(stderr, "%02X ", res);
		res = txC();
		buffer[ibuf++] = res;
	}
	fprintf(stderr, "\n");

	for(i = 0; i < ibuf; i++) {
		rxC(buffer[i]);
	}
	rxC(RFM12_L3_EOD);
	return 0;
}

// slot assignments in EEPROM
typedef struct {
	uint8_t radio_id;
	uint16_t slot;
} slot_assign_t;

typedef struct {
	uint8_t rx_fc, tx_fc; // framecounter
	uint8_t tx_state[16], tx_state_old[16]; // tx_state_old for ACK retransmission
	uint8_t rx_state[16];
} slot_t;

// TODO: move to EEP
slot_assign_t slot_assignments[] = {
	// radio_id, slot
	{1, 0}
};

slot_t slots[5]; // size: 51 bytes * elements
uint8_t aes_key[AES_EXPKEY_SIZE];
uint8_t crypto_key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}; // TODO move to flash


// find slot
slot_t *find_slot(uint8_t radio_id) {
	uint8_t i;

	for(i = 0; i < sizeof(slot_assignments) / sizeof(slot_assign_t); i++) {
		if(slot_assignments[i].radio_id == radio_id)
			return &slots[i];
	}
	return NULL;
}

void send_ack(uint8_t *tx_state) {
	RadioMessage msg;
	memset(&msg, 0, sizeof(msg));
	msg.fc = ++state.tx_fc;  // TODO state as param
	aes_encrypt();
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
		send_ack(slot->tx_state_old);
	} else if(msg->fc == slot->rx_fc + 1) {
		aes_decrypt(aes_key, &((uint8_t *) &msg)[1], &((uint8_t *) &plain)[1], slot->rx_state);
		// verify CRC
		if(radio_crc(radio_id, msg) == msg->crc16) {
			fprintf(stderr, "protocol_dispatch(): CRC correct\n");
		}
	} else {
		// possible attack
	}
}

int main() {
	RadioMessage msg, msg2;
//	int i;

	radio_init(1);

	memset(&msg, 0, sizeof(msg));
	msg.data[0] = 0x02;

	// XXX need init
	aes_key_expand(aes_key, crypto_key, AES_KEY_SRAM);

	fprintf(stderr, "radio_transmit() = %d\n", (int) radio_transmit(1, &msg));
	assert(radio_poll_receive());
	radio_receive(&msg2);
	msg2.info = 1;
/*	fprintf(stderr, "received message is:\n");
	for(i = 0; i < sizeof(RadioMessage); i++)
		fprintf(stderr, "%02X ", ((uint8_t *) &msg2)[i]);
	fprintf(stderr, "\n");*/
	assert(memcmp(&msg, &msg2, sizeof(RadioMessage)) == 0);

	msg.info = 0;
	fprintf(stderr, "radio_transmit() = %d\n", (int) radio_transmit(1, &msg));
	assert(radio_poll_receive());
	radio_receive(&msg2);
	msg2.info = 1;
	assert(memcmp(&msg, &msg2, sizeof(RadioMessage)) == 0);

	return 0;
}
