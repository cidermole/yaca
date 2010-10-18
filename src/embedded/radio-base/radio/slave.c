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
extern uint8_t radio_id, target_id;
extern radio_state_t radio_state;
extern uint8_t tx_state[16], rx_state[16], aes_key[AES_EXPKEY_SIZE];

void _radio_rxc(int16_t data) {
	static uint8_t id_in, buf_in_index = 0;
	static RadioMessage buf_in;

	if(data == RFM12_L3_EOD) {
		fprintf(stderr, PREFIX " EOF. got %d bytes\n", (int)buf_in_index);
		if(id_in != radio_id)
			return;

		fprintf(stderr, PREFIX " ID match\n");

		// decrypt
		aes_decrypt(aes_key, &((uint8_t *) &buf_in)[1], &((uint8_t *) &msg_in)[1], rx_state);

		// verify CRC
		if(radio_crc(radio_id, &msg_in) == msg_in.crc16)
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

tstatus radio_transmit(uint8_t radio_id_target, RadioMessage *msg) {
	uint8_t crypt_buf[16];

	if(radio_state != ST_IDLE)
		return PENDING;
	if(msg->info) // if we have already queued message and are done (idle)
		return SUCCESS;

	// calculate CRC (changes original message)
	msg->crc16 = radio_crc(radio_id_target, msg);

	// encrypt message
	aes_encrypt(aes_key, &((uint8_t *) msg)[1], crypt_buf, tx_state);

	memcpy(&((uint8_t *) &buf_out)[1], crypt_buf, sizeof(RadioMessage) - 1); // queue message
	msg->info = 1;
	target_id = radio_id_target;

	radio_state = ST_TX;
	RFM12_LLC_sendFrame();

	return PENDING;
}

