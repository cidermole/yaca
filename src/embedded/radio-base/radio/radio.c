#include <stdio.h>
#include <string.h>
#if defined(__AVR__)
#include <util/crc16.h>
#endif
#include "radio.h"
#include "rijndael.h"
#include "../librfm12/rfm12.h"

RadioMessage msg_in, buf_out;
uint8_t buf_out_index = 0, msg_in_full = 0;
uint8_t radio_id, target_id;
radio_state_t radio_state = ST_IDLE;
uint8_t tx_key[16], rx_key[16], tx_state[16], rx_state[16], aes_key[AES_EXPKEY_SIZE];

#define PREFIX

#if !defined(__AVR__)
uint16_t
_crc16_update(uint16_t crc, uint8_t a)
{
    int i;

    crc ^= a;
    for (i = 0; i < 8; ++i)
    {
        if (crc & 1)
        crc = (crc >> 1) ^ 0xA001;
        else
        crc = (crc >> 1);
    }

    return crc;
}
#endif

void _radio_rxc(int16_t data);
int16_t _radio_txc();

void radio_init(uint8_t radio_id_node) { // we will only receive this ID
	uint8_t _tx_key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	uint8_t _rx_key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	uint8_t _tx_state[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t _rx_state[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	// TODO for base station: random from SRAM startup content (with CRC? with xrandom() from aestable.c?)

	memcpy(tx_key, _tx_key, sizeof(_tx_key));
	memcpy(rx_key, _rx_key, sizeof(_rx_key));
	memcpy(tx_state, _tx_state, sizeof(_tx_state));
	memcpy(rx_state, _rx_state, sizeof(_rx_state));

	RFM12_LLC_registerType(&_radio_rxc, &_radio_txc);
	radio_id = radio_id_node;
}

void _radio_rxc(int16_t data) {
	static uint8_t id_in, buf_in_index = 0;
	static RadioMessage buf_in;
	uint16_t crc16 = 0xffff;
	uint8_t i;

	if(data == RFM12_L3_EOD) {
		fprintf(stderr, PREFIX " EOF. got %d bytes\n", (int)buf_in_index);
		if(id_in != radio_id)
			return;

		fprintf(stderr, PREFIX " ID match\n");

		// decrypt
		aes_key_expand(aes_key, rx_key, AES_KEY_SRAM);
		aes_decrypt(aes_key, &((uint8_t *) &buf_in)[1], &((uint8_t *) &msg_in)[1], rx_state);

		// calculate and verify CRC
		crc16 = _crc16_update(crc16, radio_id);
		for(i = 1; i < sizeof(RadioMessage) - sizeof(buf_in.crc16); i++)
			crc16 = _crc16_update(crc16, ((uint8_t *) &msg_in)[i]);

		if(crc16 == msg_in.crc16)
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

int16_t _radio_txc() {
	if(buf_out_index == 0) {
		buf_out_index++;
		return target_id;
	} else if(buf_out_index < sizeof(RadioMessage)) {
		return ((uint8_t *) &buf_out)[buf_out_index++];
	} else {
		radio_state = ST_IDLE;
		buf_out_index = 0;
		return RFM12_L3_EOD;
	}
}

tstatus radio_transmit(RadioMessage *msg, uint8_t radio_id_target) {
	uint8_t i, crypt_buf[16];
	uint16_t crc16 = 0xffff;

	if(radio_state != ST_IDLE)
		return PENDING;
	if(msg->info) // if we have already queued message and are done (idle)
		return SUCCESS;

	// calculate CRC (changes original message)
	crc16 = _crc16_update(crc16, radio_id_target);
	for(i = 1; i < sizeof(RadioMessage) - sizeof(msg->crc16); i++)
		crc16 = _crc16_update(crc16, ((uint8_t *) msg)[i]);
	msg->crc16 = crc16;

	// encrypt message
	aes_key_expand(aes_key, tx_key, AES_KEY_SRAM);
	aes_encrypt(aes_key, &((uint8_t *) msg)[1], crypt_buf, tx_state);

	memcpy(&((uint8_t *) &buf_out)[1], crypt_buf, sizeof(RadioMessage) - 1); // queue message
	msg->info = 1;
	target_id = radio_id_target;

	radio_state = ST_TX;
	RFM12_LLC_sendFrame();

	return PENDING;
}

void radio_receive(RadioMessage *msg) {
	memcpy(msg, &msg_in, sizeof(RadioMessage));
	msg_in_full = 0;
}

uint8_t radio_poll_receive() {
	return msg_in_full;
}

