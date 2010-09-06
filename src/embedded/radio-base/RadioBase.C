#include "RadioBase.h"
#include "RRadioBase.h"
#include "librfm12/rfm12.h"
#include <yaca.h>
#include <avr/io.h>

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
	RFM12_PHY_init();
	RFM12_David_init();

	sei();

	while(1) {
		if(ird >= 19) {
			temperature = (radio_data[3] - '0') * 100 + (radio_data[4] - '0') * 10 + (radio_data[6] - '0');
			voltage = (radio_data[10] - '0') * 1000 + (radio_data[12] - '0') * 100 + (radio_data[13] - '0') * 10 + (radio_data[14] - '0');

			// hack: temp sensor doesn't have '-' sign -> -0.1 deg. C ^= 535 (65535)
			// winter: -23.6 / 30.0
			// summer: -3.6 / 50.0
			if((winter && temperature > 300) || (!winter && temperature > 500))
				temperature -= 536;

		} else if(ird == 0 && !update) { // hopefully, we can now transmit without RFM12 interrupt
			yc_status(TempStatus);
			update = 1;
		}
		yc_dispatch_auto();
	}
	return 0;
}

