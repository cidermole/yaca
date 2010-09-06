#include "RadioBase.h"
#include "RRadioBase.h"
#include "librfm12/rfm12.h"
#include <yaca.h>
#include <avr/io.h>
#ifndef F_CPU
#define F_CPU 2000000UL
#endif
#include <util/delay.h>
#include <string.h>

volatile uint8_t radio_data[19*2], ird = 0, update = 1; // *2: just to be on the safe side
int16_t temperature, voltage;
uint8_t winter = 0;

void _RFM12_David_rxCallback(int data) {
	if(data == -1) {
		ird = 0;
		update = 0;
		return;
	}

	radio_data[ird++] = (uint8_t) data;
	if(ird >= sizeof(radio_data))
		ird = 19; // wait for next EOF, in the meantime, just write somewhere safe
}

int16_t _RFM12_David_txCallback(void) {
	return -1;
}

void RFM12_David_init(void) {
	RFM12_L3_Protocol_t proto;
	proto.rxCallback = &_RFM12_David_rxCallback;
	proto.txCallback = &_RFM12_David_txCallback;
	proto.ackCallback = 0;

	RFM12_LLC_registerType(TYPE_SERIAL, proto);
}

void DR(TempStatus()) {
	yc_prepare_ee(YC_EE_TEMPSTATUS_ID);
	RFM12_INT_off();
	yc_send(RadioBase, TempStatus(temperature, voltage));
	RFM12_INT_on();
}

void DM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags)) {
	if(month >= 10 || month <= 5)
		winter = 1;
	else
		winter = 0;
}

int main() {
	sei();
	RFM12_David_init();
	RFM12_PHY_init();

	while(1) {
		if(ird == 0 && !update) { // hopefully, we can now transmit without RFM12 interrupt
			temperature = ((int16_t)(radio_data[0] - '0')) * 100;
			temperature += (radio_data[1] - '0') * 10;
			temperature += (radio_data[3] - '0');

			voltage = ((uint16_t)(radio_data[7] - '0')) * 1000;
			voltage += ((uint16_t)(radio_data[9] - '0')) * 100;
			voltage += (radio_data[10] - '0') * 10;
			voltage += (radio_data[11] - '0');

			// hack: temp sensor doesn't have '-' sign -> -0.1 deg. C ^= 535 (65535)
			// winter: -23.6 / 30.0
			// summer: -3.6 / 50.0
			if((winter && temperature > 300) || (!winter && temperature > 500))
				temperature -= 536;

			yc_status(TempStatus);
			update = 1;
		}
		yc_dispatch_auto();
	}
	return 0;
}

