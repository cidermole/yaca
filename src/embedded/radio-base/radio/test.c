#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../librfm12/rfm12.h"
#include "radio.h"
#include "rijndael.h"

RFM12_L3_rxCallback *rxC = 0;
RFM12_L3_txCallback *txC = 0;

uint8_t buffer[128], ibuf = 0;
uint8_t test_aes_key[AES_EXPKEY_SIZE], test_aes_state[16];

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

/*	for(i = 0; i < ibuf; i++) {
		rxC(buffer[i]);
	}
	rxC(RFM12_L3_EOD);*/
	return 0;
}

void test_rxc(uint8_t *buf, uint8_t count) {
	int i;

	for(i = 0; i < count; i++) {
		rxC(buf[i]);
	}
	rxC(RFM12_L3_EOD);
}

int main() {
	RadioMessage msg;
	uint8_t buf[sizeof(RadioMessage) + 1];
	uint8_t _key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

	radio_init(1);
	aes_key_expand(test_aes_key, _key, AES_KEY_SRAM);
	memset(&test_aes_state, 0, sizeof(test_aes_state));

	memset(&msg, 0, sizeof(msg));
	msg.data[0] = 0x02;
	msg.fc = 1;
	buf[0] = 2; // radio_id
	buf[1] = msg.fc;
	msg.crc16 = radio_crc(buf[0] /* radio_id */, &msg);
	aes_encrypt(test_aes_key, &((uint8_t *) &msg)[2], &((uint8_t *) buf)[2], test_aes_state);
	test_rxc(buf, sizeof(buf));
	return 0;
}
