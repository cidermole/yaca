#include <string.h>
#include <util/crc16.h>
#include "radio.h"
#include "rijndael.h"
#include "../librfm12/rfm12.h"

RadioMessage msg_in, buf_out;
uint8_t buf_out_index = 0, msg_in_full = 0;
uint8_t our_radio_id, target_id;
radio_state_t radio_state = ST_IDLE;
uint8_t aes_key[AES_EXPKEY_SIZE];


void _radio_rxc(int16_t data);
int16_t _radio_txc();

uint16_t radio_crc(uint8_t radio_id, RadioMessage *msg) {
	uint16_t crc16 = 0xffff;
	uint8_t i;

	crc16 = _crc16_update(crc16, radio_id);
	for(i = 1; i < sizeof(RadioMessage) - sizeof(msg->crc16); i++)
		crc16 = _crc16_update(crc16, ((uint8_t *) msg)[i]);

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

void _radio_rxc(int16_t data) {
	static uint8_t id_in, buf_in_index = 0;
	static RadioMessage buf_in;

	if(data == RFM12_L3_EOD) {
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

void radio_receive(RadioMessage *msg) {
	memcpy(msg, &msg_in, sizeof(RadioMessage));
	msg_in_full = 0;
}

uint8_t radio_poll_receive() {
	return msg_in_full;
}

