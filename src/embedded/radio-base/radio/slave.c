#include <stdio.h>
#include <string.h>
#if defined(__AVR__)
#include <util/crc16.h>
#endif
#include "radio.h"
#include "rijndael.h"
#include "../librfm12/rfm12.h"

#define PREFIX

extern RadioMessage msg_in, buf_out;
extern uint8_t buf_out_index, msg_in_full;
extern uint8_t our_radio_id, target_id;
extern radio_state_t radio_state;
extern uint8_t aes_key[AES_EXPKEY_SIZE];
uint8_t tx_state[16], rx_state[16], rx_fc = 0, tx_fc = 0, _radio_sync = 0;

void _radio_rxc(int16_t data);
int16_t _radio_txc();


void radio_init(uint8_t radio_id_node) { // we will only receive this ID
	// TODO: store in EEPROM
	uint8_t _tx_key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	aes_key_expand(aes_key, _tx_key, AES_KEY_SRAM);

	RFM12_LLC_registerType(&_radio_rxc, &_radio_txc);
	our_radio_id = radio_id_node;
	_radio_sync = 0;
}

void radio_slave_resync() {
	RadioMessage msg;

	while(radio_state != ST_IDLE);

	// everything else except fc and radio_id doesn't matter atm
	msg.fc = 0xFF;
	target_id = our_radio_id;

	radio_state = ST_TX;
	RFM12_LLC_sendFrame();
}

void protocol_dispatch(uint8_t radio_id, RadioMessage *msg) {
	uint8_t state[16];

	// not for us? (MSB is AES state transmission indicator)
	if((radio_id & 0x7F) != our_radio_id)
		return;

	// AES state transmission and we are waiting for it?
	if((radio_id & 0x80) && _radio_sync == 0) {
		rx_fc = 0;
		tx_fc = 0;
		memcpy(tx_state, &((uint8_t *) msg)[1], sizeof(tx_state));
		memcpy(rx_state, &((uint8_t *) msg)[1], sizeof(rx_state));
		_radio_sync = 1;
		return;
	}

	// frame counter wrong? (possible attack)
	if(msg->fc != rx_fc + 1 && !(rx_fc == 0xFE && msg->fc == 0))
		return;

	// decrypt
	memcpy(state, rx_state, sizeof(state)); // backup state
	aes_decrypt(aes_key, &((uint8_t *) msg)[2], &((uint8_t *) &msg_in)[2], rx_state);
	msg_in.fc = msg->fc;

	// verify CRC
	if(radio_crc(radio_id, &msg_in) == msg_in.crc16) {
		fprintf(stderr, PREFIX " CRC match\n");
		if(++rx_fc == 0xFF) // avoid special code FF
			rx_fc = 0;
		msg_in_full = 1;
	} else {
		memcpy(rx_state, state, sizeof(state)); // restore state
	}
}

tstatus radio_transmit(uint8_t radio_id, RadioMessage *msg) {
	if(radio_state != ST_IDLE)
		return PENDING;
	if(msg->info) // if we have already queued message and are done (idle)
		return SUCCESS;

	// set frame counter, calculate CRC (changes original message)
	if(++tx_fc == 0xFF) // avoid special code FF
		tx_fc = 0;
	msg->fc = tx_fc;
	msg->crc16 = radio_crc(radio_id, msg);

	// encrypt and queue message
	aes_encrypt(aes_key, &((uint8_t *) msg)[1], &((uint8_t *) &buf_out)[1], tx_state);
	msg->info = 1;
	target_id = radio_id;

	radio_state = ST_TX;
	RFM12_LLC_sendFrame();

	return PENDING;
}

