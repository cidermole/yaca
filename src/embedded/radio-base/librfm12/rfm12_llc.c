#include <util/crc16.h>

#include "hamming.h"
#include "rfm12_llc.h"
#include "rfm12_mac.h"

typedef uint8_t RFM12_LLC_CRC_t;

uint8_t _RFM12_LLC_deviceID;

volatile struct {
	RFM12_LLC_FrameHeader_t header;
	uint8_t					buffer[RFM12_MAC_MAX_DATA_LENGTH-sizeof(RFM12_LLC_FrameHeader_t)+sizeof(RFM12_LLC_CRC_t)];
} _RFM12_LLC_rx;

volatile struct {
	RFM12_LLC_FrameHeader_t header;
	RFM12_LLC_CRC_t			crc;
} _RFM12_LLC_tx;

RFM12_L3_Protocol_t RFM12_LLC_protocols[16];

uint8_t RFM12_LLC_sendFrame(RFM12_ProtocolID_t proto, uint8_t receiver, bool requireACK)
{
	_RFM12_LLC_tx.header.receiver = receiver;
	_RFM12_LLC_tx.header.sender = _RFM12_LLC_deviceID;
	_RFM12_LLC_tx.header.requireACK = requireACK;
	// TODO: if this is direct replay -> ACK = true
	_RFM12_LLC_tx.header.protID = proto;
	_RFM12_LLC_tx.crc = 0;

	RFM12_MAC_startCtrlTransmission();
	return 0;
}

// Take care, will be called from interrupt
bool RFM12_LLC_receiveCallback(uint8_t data)
{
	static uint8_t counter = 0, hamming;
	RFM12_L3_rxCallback* rxCallback;

/*	uart_putc('&');
	uart_put_hex(data);
	uart_puts_P(PSTR("\r\n"));*/

	if(data == RFM12_MAC_EOF) {
//		uart_puts_P(PSTR("EOF\r\n"));
		counter = 0;
		return false;
	}

	if(counter & 0x01) {
		hamming |= Hamming_decLow(data);
		((uint8_t*)&_RFM12_LLC_rx)[counter>>1] = hamming;

		if(counter < sizeof(RFM12_LLC_FrameHeader_t) * 2)
			((uint8_t*)&(_RFM12_LLC_tx.header))[counter >> 1] = hamming;
		else {
/*			uart_puts_P(PSTR("Callback. protID = 0x"));
			uart_put_hex(_RFM12_LLC_tx.header.protID);
			uart_puts_P(PSTR(" data = 0x"));
			uart_put_hex(hamming);
			uart_puts_P(PSTR(" receiver = 0x"));
			uart_put_hex(_RFM12_LLC_tx.header.receiver);
			uart_puts_P(PSTR("\r\n"));*/
			rxCallback = RFM12_LLC_protocols[_RFM12_LLC_tx.header.protID].rxCallback;
			if(rxCallback)
				rxCallback(hamming);
		}
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

	if(counter < RFM12_MAC_MAX_DATA_LENGTH) {
		if((counter & 0x01) == 0x00) {							// Hamming high or low byte
			if(counter < sizeof(RFM12_LLC_FrameHeader_t))
				data = ((uint8_t*)&(_RFM12_LLC_tx.header))[counter >> 1];
			else
				data = RFM12_LLC_protocols[_RFM12_LLC_tx.header.protID].txCallback();	// TODO: check if callback is zero

			_RFM12_LLC_tx.crc = _crc_ibutton_update(_RFM12_LLC_tx.crc, data);
			counter++;
			if(data == RFM12_L3_EOD) {
				counter = RFM12_MAC_MAX_DATA_LENGTH-1;
				data = _RFM12_LLC_tx.crc;
			}
			return Hamming_encHigh(data);
		} else {
			counter++;
			return Hamming_encLow(data);
		}
	}
	counter = 0;
	return RFM12_MAC_EOF;
}

void RFM12_LLC_registerType(RFM12_ProtocolID_t typeID, RFM12_L3_Protocol_t proto)
{
	RFM12_LLC_protocols[typeID & 0x0f] = proto;
}

