#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../librfm12/rfm12.h"
#include "radio.h"

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
}

int main() {
	RadioMessage msg, msg2;
//	int i;

	radio_init(1);

	memset(&msg, 0, sizeof(msg));
	msg.data[0] = 0x02;

	fprintf(stderr, "radio_transmit() = %d\n", (int) radio_transmit(&msg, 1));
	assert(radio_poll_receive());
	radio_receive(&msg2);
	msg2.info = 1;
/*	fprintf(stderr, "received message is:\n");
	for(i = 0; i < sizeof(RadioMessage); i++)
		fprintf(stderr, "%02X ", ((uint8_t *) &msg2)[i]);
	fprintf(stderr, "\n");*/
	assert(memcmp(&msg, &msg2, sizeof(RadioMessage)) == 0);

	msg.info = 0;
	fprintf(stderr, "radio_transmit() = %d\n", (int) radio_transmit(&msg, 1));
	assert(radio_poll_receive());
	radio_receive(&msg2);
	msg2.info = 1;
	assert(memcmp(&msg, &msg2, sizeof(RadioMessage)) == 0);

	return 0;
}
