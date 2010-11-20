#include <util/delay.h>
#include <avr/interrupt.h>
#include "spi.h"
#include "rfm12_mac.h"
#include "rfm12_phy.h"

typedef union conv_ {
	unsigned int w;
	unsigned char b[2];
} CONVERTW;

static volatile RFM12_PHY_State_t _RFM12_PHY_state = RFM12_LISTEN;

static uint16_t _RFM12_trans(uint16_t wert)
{
	uint16_t timeout = 0;
	uint8_t sreg;
	CONVERTW val;
	val.w=wert;
	sreg = SREG;
	cli();
	RFM12_select();
	SPI_trans(val.b[1], timeout);
	SPI_trans(val.b[0], timeout);
	RFM12_unselect();
	SREG = sreg;
	return val.w;
}

static bool _RFM12_waitForFIFO(void)
{
	uint16_t status, timeout = 1000;
	while(!((status=RFM12_PHY_getStatus()) & 0x8000) && --timeout);

	return timeout != 0;
}

void RFM12_PHY_init(void)
{
	SPI_confMaster();	// Configure PINs as SPI master
/*	SPI_master();		// Master mode
	SPI_disableINT();	// Disable SPI interrupt
	SPI_dordMSB();		// Data Order: MSB first
	SPI_cpolIdleLow();	// Clock Polarity: SCK low when idle
	SPI_cphaSampRise(); // Clock Phase: sample on rising SCK edge
	SPI_setFrequency(); // SPI frequency: ~ 3 MHz
	SPI_enable();*/

	_delay_ms(200);				// wait until POR done
	_RFM12_trans(0xC0E0);			// AVR CLK: 10MHz
	_RFM12_trans(0x80E7);			// Enable FIFO, b1/b0 = 868 MHz (RF12 datasheet p. 14)
	_RFM12_trans(0xC2AC);			// Data Filter: internal
	_RFM12_trans(0xCA81);			// Set FIFO mode
	_RFM12_trans(0xE000);			// disable wakeuptimer
	_RFM12_trans(0xC800);			// disable low duty cycle
	_RFM12_trans(0xC4F7);			// AFC settings: autotuning: -10kHz...+7,5kHz

	_delay_ms(5);
	RFM12_PHY_setCenterFrequency(RFM12FREQ868(868));
	RFM12_PHY_setReceiverBandwidth(VDI_fast, RxBW200, LNA_0, RSSI_97); // was: LNA_6 (-6 dB), RSSI -79 dB
	RFM12_PHY_setBaudrate(9600);
	RFM12_PHY_setTransmitPower(PWRdB_0, TxBW105); // was: PWRdB_3 (-3 dB tx power)
	RFM12_PHY_modeRX();
	RFM12_PHY_getStatus();

	RFM12_INT_init();
	RFM12_INT_on();
}

inline uint16_t RFM12_PHY_getStatus(void)
{
	return _RFM12_trans(0x0000);
}

void RFM12_PHY_modeRX(void)
{
	_RFM12_trans(0x82C8);				// RX on
	_delay_ms(1);						// TODO: delay nötig?
	_RFM12_trans(0xCA81);				// set FIFO mode
	_RFM12_trans(0xCA83);				// enable FIFO: sync word search

	RFM12_PHY_getStatus();				// clear pending INT
	_RFM12_trans(0xB000);				// empty FIFO

	_RFM12_PHY_state = RFM12_LISTEN;
}

void RFM12_PHY_modeTX(void)
{
	_RFM12_trans(0x8208);			// RX off
	_RFM12_trans(0x8238);			// TX on
	_RFM12_PHY_state = RFM12_TX;
}

void RFM12_PHY_setPowerManagement(bool enRX, bool enBB, bool startTX, bool enSynth, bool enOSC, bool enBat, bool enWkT, bool clkOff)
{
	_RFM12_trans(0x8200 | (enRX << 7) | (enBB << 6) | (startTX << 5) | (enSynth << 4) | (enOSC << 3) | (enBat << 2) | (enWkT << 1) | (clkOff << 0));
}

void RFM12_PHY_setDataFilter(bool autoLock, bool fastMode, bool analog, uint8_t dqdThres)
{
	_RFM12_trans(0xC228 | (autoLock << 7) | (fastMode << 6) | (analog << 4) | (dqdThres & 0x07));
}

void RFM12_PHY_setReceiverBandwidth(RFM12_VDI_t vdi, RFM12_RxBW_t bandwidth, RFM12_GAIN_t gain, RFM12_RSSI_t drssi)
{
	_RFM12_trans(0x9400 | ((vdi&3)<<8) | ((bandwidth&7)<<5) | ((gain&3)<<3) | (drssi&7));
}

void RFM12_PHY_setCenterFrequency(uint16_t freq)
{
	if (freq<96)
		freq=96;
	else if (freq>3903)
		freq=3903;
	_RFM12_trans(0xA000|freq);
}

void RFM12_PHY_setBaudrate(uint32_t baud)
{
	if (baud<664) baud=664;

	if (baud<5400)					// Baudrate= 344827,58621/(R+1)/(1+CS*7)
		_RFM12_trans(0xC680|((43104/baud)-1));	// R=(344828/8)/Baud-1
	else
		_RFM12_trans(0xC600|((344828UL/baud)-1));	// R=344828/Baud-1
}

void RFM12_PHY_setTransmitPower(RFM12_Power_t power, RFM12_TxDev_t deviation)
{
	_RFM12_trans(0x9800 | ((deviation&15)<<4) | (power&7));
}

void RFM12_fifoWrite(uint8_t data) {
	_RFM12_waitForFIFO();
	_RFM12_trans(0xB800 | data);
}

static inline uint8_t RFM12_fifoRead() {
	return _RFM12_trans(0xB000);
}

bool RFM12_PHY_busy(void)
{
	return _RFM12_PHY_state != RFM12_LISTEN;
}

void RFM12_PHY_timer(unsigned char mantissa, unsigned char exponent)
{
	_RFM12_trans(0xE000 | ((exponent & 0x1F) << 8) | mantissa); // set timer interval
	_RFM12_trans(0x8203); // enable timer
}

RFM12_INT_vect()
{
	uint8_t buf;
	uint16_t status = RFM12_PHY_getStatus();

	if(status & RFM12_FFIT_RGIT) {		// FIFO trigger
		switch(_RFM12_PHY_state) {
		case RFM12_LISTEN:
			_RFM12_PHY_state = RFM12_RX;
		case RFM12_RX:
			buf = RFM12_fifoRead();
			if(		!(status & RFM12_RSSI_ATS) ||			// signal lost
					RFM12_MAC_receiveCallback(buf) ) {		// end of frame
				RFM12_PHY_modeRX();
				_RFM12_PHY_state = RFM12_LISTEN;
			}
			break;
		case RFM12_TX:
			_RFM12_trans(0xB800 | RFM12_MAC_transmitCallback());
			break;
		}
	}
	if(status & RFM12_FFOV_RGUR) {		// FIFO over-/underflow
		 // TODO: else if(status & ...)
	}
	if(status & RFM12_POR) {			// Power On Reset
		 // TODO: else if(status & ...)
	}
	if(status & RFM12_WKUP) {			// Wake-Up trigger
		 // TODO: else if(status & ...)
	}
	if(status & RFM12_LBD) {			// Low supply voltage
		 // TODO: else if(status & ...)
	}
	if(status & RFM12_EXT) {			// External interrupt
		 // TODO: else if(status & ...)
	}
}
