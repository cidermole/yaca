#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "rfm12_mac.h"
#include "rfm12_phy.h"
//#include "../led.h"

const uint8_t	_RFM12_MAC_preamble[] = {0xAA, 0xAA, 0xAA, 0x2D, 0xD4};


typedef struct {
	uint16_t ctrlFreq;
	uint16_t burstFreq;
} RFM12_MAC_Channel_t;

// TODO: adapt other values for 868 MHz
RFM12_MAC_Channel_t PROGMEM _RFM12_MAC_channels[] = {
		{RFM12FREQ868(868.0), RFM12FREQ433(431.4)},	//  0
		{RFM12FREQ433(431.5), RFM12FREQ433(432.0)},	//  1
		{RFM12FREQ433(432.0), RFM12FREQ433(432.4)},	//  2
		{RFM12FREQ433(432.5), RFM12FREQ433(433.0)},	//  3
		{RFM12FREQ433(433.0), RFM12FREQ433(433.4)},	//  4
		{RFM12FREQ433(433.5), RFM12FREQ433(434.0)},	//  5
		{RFM12FREQ433(434.0), RFM12FREQ433(434.4)},	//  6
		{RFM12FREQ433(434.5), RFM12FREQ433(435.0)},	//  7
		{RFM12FREQ433(435.0), RFM12FREQ433(435.4)},	//  8
		{RFM12FREQ433(435.5), RFM12FREQ433(436.0)},	//  9
		{RFM12FREQ433(436.0), RFM12FREQ433(436.4)},	// 10
		{RFM12FREQ433(436.5), RFM12FREQ433(437.0)},	// 11
		{RFM12FREQ433(437.0), RFM12FREQ433(437.4)},	// 12
		{RFM12FREQ433(437.5), RFM12FREQ433(437.0)},	// 13
		{RFM12FREQ433(438.0), RFM12FREQ433(438.4)},	// 14
		{RFM12FREQ433(438.5), RFM12FREQ433(439.0)}	// 15
};

struct {
	RFM12_MAC_Channel_t*	channel;
	bool					burstMode;
} _RFM12_MAC = { _RFM12_MAC_channels + 0, false };

volatile RFM12_MAC_State_t _RFM12_MAC_state;

bool RFM12_MAC_mediaBusy(void)
{
	return 	RFM12_PHY_busy() || (RFM12_PHY_getStatus() & RFM12_RSSI_ATS);
	// TODO: long term media observation
}

void RFM12_MAC_setChannel(uint8_t channel)
{
	while(RFM12_PHY_busy());	// wait for media to become free

	_RFM12_MAC.channel = &_RFM12_MAC_channels[channel & 0x0f];
	_RFM12_MAC.burstMode = false;
	RFM12_PHY_setCenterFrequency(pgm_read_word(&(_RFM12_MAC.channel->ctrlFreq)));
	RFM12_PHY_setBaudrate(RFM12_MAC_CTRL_BAUD);
	RFM12_PHY_setReceiverBandwidth(VDI_fast, RFM12_MAC_CTRL_BW, LNA_6, RSSI_79);
	RFM12_PHY_setTransmitPower(PWRdB_6, RFM12_MAC_CTRL_DEV);
}

void RFM12_MAC_startCtrlTransmission(void)
{
	while(RFM12_PHY_busy());	// wait for media to become free

//	LED_on();
	RFM12_PHY_modeTX();
}

void RFM12_MAC_endCtrlTransmission(void)
{
	RFM12_PHY_modeRX();
	_RFM12_MAC_state = RFM12_MAC_TX_WAIT;
//	_RFM12_MAC_startTimer(RFM12_MAC_TX_WAIT_TIME);
//	LED_off();
}

void RFM12_MAC_init(void)
{
	// TODO: init timer

}

bool RFM12_MAC_receiveCallback(uint8_t data)
{
	RFM12_LLC_receiveCallback(data);

	if(data == RFM12_MAC_EOF)
		return true;

	return false;
}

uint8_t RFM12_MAC_transmitCallback(void)
{
	static uint8_t counter = 0;

	if(counter < sizeof(_RFM12_MAC_preamble)) {											// HEADER
		return _RFM12_MAC_preamble[counter++];
	} else if(counter < (sizeof(_RFM12_MAC_preamble) + RFM12_MAC_MAX_DATA_LENGTH)) {	// DATA
		uint8_t data = RFM12_LLC_transmitCallback();
		counter++;
		if(data != RFM12_MAC_EOF)			// if not end of data
			return data;
		counter = (sizeof(_RFM12_MAC_preamble) + RFM12_MAC_MAX_DATA_LENGTH);
		return RFM12_MAC_EOF;
	} else if(counter == (sizeof(_RFM12_MAC_preamble) + RFM12_MAC_MAX_DATA_LENGTH)) {   // EOF
		counter++;
		return RFM12_MAC_EOF; // TODO: additional 0xAA for current debugging prog, check
	} else {																			// EOF
		counter = 0;
		RFM12_MAC_endCtrlTransmission();
		return RFM12_MAC_EOF;
	}
}

