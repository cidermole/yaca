#include <util/crc16.h>

#include "hamming.h"
#include "rfm12_llc.h"
#include "rfm12_mac.h"

RFM12_L3_rxCallback *RFM12_LLC_protocol_rxCallback = 0;
RFM12_L3_txCallback *RFM12_LLC_protocol_txCallback = 0;

uint8_t RFM12_LLC_sendFrame()
{
	RFM12_MAC_startCtrlTransmission();
	return 0;
}

// Take care, will be called from interrupt
bool RFM12_LLC_receiveCallback(uint8_t data)
{
	static uint8_t counter = 0, hamming;

	if(data == RFM12_MAC_EOF) {
		counter = 0;
		if(RFM12_LLC_protocol_rxCallback)
			RFM12_LLC_protocol_rxCallback(-1);
		return false;
	}

	if(counter & 0x01) {
		hamming |= Hamming_decLow(data);

		if(RFM12_LLC_protocol_rxCallback)
			RFM12_LLC_protocol_rxCallback(hamming);
	} else {
		hamming = Hamming_decHigh(data);
	}
	counter++;

	return false;
}

// Take care, will be called from interrupt
uint8_t RFM12_LLC_transmitCallback(void)
{
	static uint8_t counter = 0;
	static int16_t data;

	if((counter & 0x01) == 0x00) {							// Hamming high or low byte
		if(RFM12_LLC_protocol_txCallback)
			data = RFM12_LLC_protocol_txCallback();

		counter++;
		if(data == RFM12_L3_EOD) {
			counter = 0;
			return RFM12_MAC_EOF;
		}
		return Hamming_encHigh(data);
	} else {
		counter++;
		return Hamming_encLow(data);
	}
}

void RFM12_LLC_registerType(RFM12_L3_rxCallback *rxCallback, RFM12_L3_txCallback *txCallback)
{
	RFM12_LLC_protocol_rxCallback = rxCallback;
	RFM12_LLC_protocol_txCallback = txCallback;
}

