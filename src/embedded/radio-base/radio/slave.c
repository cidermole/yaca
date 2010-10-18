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
uint8_t tx_state[16], rx_state[16];

void _radio_rxc(int16_t data);
int16_t _radio_txc();


void radio_init(uint8_t radio_id_node) { // we will only receive this ID
	uint8_t _tx_key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	uint8_t _tx_state[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t _rx_state[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	// TODO for base station: random from SRAM startup content (with CRC? with xrandom() from aestable.c?)

	memcpy(tx_state, _tx_state, sizeof(_tx_state));
	memcpy(rx_state, _rx_state, sizeof(_rx_state));
	aes_key_expand(aes_key, _tx_key, AES_KEY_SRAM);

	RFM12_LLC_registerType(&_radio_rxc, &_radio_txc);
	our_radio_id = radio_id_node;
}

void _radio_rxc(int16_t data) {
	static uint8_t id_in, buf_in_index = 0;
	static RadioMessage buf_in;

	if(data == RFM12_L3_EOD) {
		fprintf(stderr, PREFIX " EOF. got %d bytes\n", (int)buf_in_index);
		if(id_in != our_radio_id)
			return;

		fprintf(stderr, PREFIX " ID match\n");

		// decrypt
		aes_decrypt(aes_key, &((uint8_t *) &buf_in)[1], &((uint8_t *) &msg_in)[1], rx_state);

		// verify CRC
		if(radio_crc(id_in, &msg_in) == msg_in.crc16)
		{
			fprintf(stderr, PREFIX " CRC match\n");
			msg_in_full = 1;
		}
		buf_in_index = 0;
		radio_state = ST_IDLE;
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
