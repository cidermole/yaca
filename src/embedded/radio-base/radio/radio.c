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
uint8_t tx_state[16], rx_state[16], aes_key[AES_EXPKEY_SIZE];

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
	uint8_t _tx_state[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t _rx_state[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	// TODO for base station: random from SRAM startup content (with CRC? with xrandom() from aestable.c?)

	memcpy(tx_state, _tx_state, sizeof(_tx_state));
	memcpy(rx_state, _rx_state, sizeof(_rx_state));
	aes_key_expand(aes_key, _tx_key, AES_KEY_SRAM);

	RFM12_LLC_registerType(&_radio_rxc, &_radio_txc);
	radio_id = radio_id_node;
}

uint16_t radio_crc(uint8_t radio_id, RadioMessage *msg) {
	uint16_t crc16 = 0xffff;
	uint8_t i;

	crc16 = _crc16_update(crc16, radio_id);
	for(i = 1; i < sizeof(RadioMessage) - sizeof(msg->crc16); i++)
		crc16 = _crc16_update(crc16, ((uint8_t *) &msg)[i]);

	return crc16;
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

void radio_receive(RadioMessage *msg) {
	memcpy(msg, &msg_in, sizeof(RadioMessage));
	msg_in_full = 0;
}

uint8_t radio_poll_receive() {
	return msg_in_full;
}

