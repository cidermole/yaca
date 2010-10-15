/*
 ******************************************************************************
 * RFM12 Protokoll Stack
 * Copyright (C) 2008  Manuel Stahl (thymythos@web.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ******************************************************************************
 */
/*
 *******************************************************************************
 *  RFM12 Library
 *
 *  Developer:
 *     Benedikt K.
 *     Juergen Eckert
 *     Manuel Stahl
 *
 *  Version: 3.0.0
 *
 *******************************************************************************
 */
#ifndef RFM12_H_
#define RFM12_H_

#include <stdbool.h>
#include <stdint.h>
#include "rfm12_stack.h"

/*********************** CONFIG *************************************/

#define RFM12_select() 			PORTB &= ~(1 << PB1)
#define RFM12_unselect() 		PORTB |= (1 << PB1)

// configure INT1 on falling edge
#define RFM12_INT_init()		MCUCR &= ~(_BV(ISC11) | _BV(ISC10))
#define RFM12_INT_on()			GICR |= _BV(INT1)
#define RFM12_INT_off()			GICR &= ~_BV(INT1)
#define RFM12_INT_vect()		ISR(INT1_vect)

/*********************** DEFINITIONS ********************************/

typedef enum RFM12_VDI {
	VDI_fast,
	VDI_mid,
	VDI_slow,
	VDI_always
} RFM12_VDI_t;

typedef enum RFM12_RxBW {
	RxBWreserved = 0,
	RxBW400 =	1,
	RxBW340	=	2,
	RxBW270	=	3,
	RxBW200	=	4,
	RxBW134	=	5,
	RxBW67  =	6
} RFM12_RxBW_t;

typedef enum RFM12_GAIN {
	LNA_0,
	LNA_6,
	LNA_14,
	LNA_20
} RFM12_GAIN_t;

typedef enum RFM12_RSSI {
	RSSI_103,
	RSSI_97,
	RSSI_91,
	RSSI_85,
	RSSI_79,
	RSSI_73,
	RSSI_67,
	RSSI_61
} RFM12_RSSI_t;

typedef enum RFM12_TxDev {
	TxBW15,
	TxBW30,
	TxBW45,
	TxBW60,
	TxBW75,
	TxBW90,
	TxBW105,
	TxBW120,
	TxBW135,
	TxBW150,
	TxBW165,
	TxBW180,
	TxBW195,
	TxBW210,
	TxBW225,
	TxBW240
} RFM12_TxDev_t;

typedef enum RFM12_Power {
	PWRdB_0,
	PWRdB_3,
	PWRdB_6,
	PWRdB_9,
	PWRdB_12,
	PWRdB_15,
	PWRdB_18,
	PWRdB_21
} RFM12_Power_t;

typedef struct RFM12_Status {
	uint16_t FFIT_RGIT:1;	// (RGIT = TX-Register ist bereit neue Daten zu senden ... kann mit dem TX-Register gelöscht werden)
    						// (FFIT = Die Anzahl der Datenbits im FIFO-Puffer hat das eingestellte Limit erreicht. Kann mit einer der FIFO-Lesemethoden gelöscht werden)
	uint16_t POR:1;			// (PowerOnReset)
	uint16_t FFOV_RGUR:1;	// (RGUR = Der Datenstrom beim Senden ist abgerissen, da nicht schnell genug Daten nachgeladen wurden)
     						// (FFOV = Der RX-FIFO ist übergelaufen)
	uint16_t WKUP:1;
	uint16_t EXT:1;
	uint16_t LBD:1;
	uint16_t FFBM:1; 		// (Der FIFO-Puffer ist leer)
	uint16_t RSSI_ATS:1; 	// (ATS = )(RSSI = Die Signalstärke ist über dem eingestelltem Limit)
	uint16_t DQD:1;
	uint16_t CRL:1;
	uint16_t ATGL:1;
	uint16_t OFFS:5;
} RFM12_Status_t;

#define RFM12_FFIT_RGIT	_BV(15)	// (RGIT = TX-Register ist bereit neue Daten zu senden ... kann mit dem TX-Register gelöscht werden)
								// (FFIT = Die Anzahl der Datenbits im FIFO-Puffer hat das eingestellte Limit erreicht. Kann mit einer der FIFO-Lesemethoden gelöscht werden)
#define RFM12_POR		_BV(14)	// (PowerOnReset)
#define RFM12_FFOV_RGUR	_BV(13)	// (RGUR = Der Datenstrom beim Senden ist abgerissen, da nicht schnell genug Daten nachgeladen wurden)
 								// (FFOV = Der RX-FIFO ist übergelaufen)
#define RFM12_WKUP		_BV(12)
#define RFM12_EXT		_BV(11)
#define RFM12_LBD		_BV(10)
#define RFM12_FFBM 		_BV(9)	// (Der FIFO-Puffer ist leer)
#define RFM12_RSSI_ATS	_BV(8) 	// (ATS = )(RSSI = Die Signalstärke ist über dem eingestelltem Limit)
#define RFM12_DQD		_BV(7)
#define RFM12_CRL		_BV(6)
#define RFM12_ATGL		_BV(5)
#define RFM12_OFFS		_BV(0)


#define RFM12FREQ433(freq)  ((freq-430.0)/0.0025)   // macro for calculating frequency value out of frequency in MHz
#define RFM12FREQ868(freq)  ((freq-860.0)/0.005)

/**
 *******************************************************************************
 * Initialize the library. Must be called before any other library function.
 *******************************************************************************
 */
void RFM12_PHY_init(void);

/**
 *
 */
extern inline void RFM12_PHY_setPowerManagement(bool enRX, bool enBB, bool startTX, bool enSynth, bool enOSC, bool enBat, bool enWkT, bool clkOff);

/**
 *******************************************************************************
 * Configure the data filter.
 *
 * @param autoLock	Baudratenregenerator schaltet automatisch in den langsamen Modus,
 *					sobald er einen Takt erkannt hat.
 * @param fastMode	schneller/langsamer Modus
 * @param analog	Typ des Datenfilters (0=DigitalFilter / 1=AnalogFilter)
 * @param dqdThres	Bestimmt den Schwellwert, ab dem ein Signal als gut empfunden wird,
 *					und der Empfänger dieses weiterverarbeiten soll.
 * 					DQD (data quality detection) zählt die "Spikes" des ungefilterten
 * 					Signals, und bestimmt darüber die Qualität der Daten.
 *******************************************************************************
*/
void RFM12_PHY_setDataFilter(bool autoLock, bool fastMode, bool analog, uint8_t dqdThres);

/**
 *******************************************************************************
 * Configure the receiver.
 * @param vdi		(Valid Data Indicator). Definiert die Reaktionsgeschwindigkeit,
 * 					mit der bestimmt wird, ob ein Signal korrekt ist, oder nicht.
 * @param bandwidth	Bestimmt die Bandbreite des Empfängers in KHz (KiloHertz)
 * @param gain		(LNA-Gain) Verstärkungsfaktor des Rauscharmen-Eingangs-Signal-
 * 					Verstärkers (LNA Low Noise Amplifier). Werte in dBm.
 * @param drssi		(DRSSI = Digital Received Signal Strength Indication)
 * 					Minimale Empfangssignalfeldstärke in dBm (wird zu gain addiert).
 *******************************************************************************
 */
void RFM12_PHY_setReceiverBandwidth(RFM12_VDI_t vdi, RFM12_RxBW_t bandwidth, RFM12_GAIN_t gain, RFM12_RSSI_t drssi);

/**
 *******************************************************************************
 * Configure the FSK center frequency.
 * @param freq	use the RFM12FREQ433 makro to set the frequency in MHz
 *******************************************************************************
 */
void RFM12_PHY_setCenterFrequency(uint16_t freq);

/**
 *******************************************************************************
 * Set the baud rate.
 * @param baud	the baud rate (1200 - 115200)
 *******************************************************************************
 */
void RFM12_PHY_setBaudrate(uint32_t baud);

/**
 *******************************************************************************
 * Configure the transmitter.
 * @param power		Bestimmt die relative Ausgangsleistung des Senders in dBm
 * @param deviation	Bestimmt den Frequenzabstand des High- und Low-Wertes bei
 * 					der Ubertragung im FSK-Betrieb in kHz.
 *******************************************************************************
 */
void RFM12_PHY_setTransmitPower(RFM12_Power_t power, RFM12_TxDev_t deviation);

/**
 *******************************************************************************
 * Start the RFM12 transmitter.
 *******************************************************************************
 */
void RFM12_PHY_modeTX(void);

/**
 *******************************************************************************
 * Put the RFM12 in listen mode.
 *******************************************************************************
 */
void RFM12_PHY_modeRX(void);

/**
 *******************************************************************************
 * Is RFM12 busy transmitting or receiving.
 * @return true if busy
 *******************************************************************************
 */
bool RFM12_PHY_busy(void);

/**
 *******************************************************************************
 * Get the status of the RFM12. Can be cast to <code>RFM12_Status_t</code>.
 * @return the status bits of the RFM12
 *******************************************************************************
 */
extern inline uint16_t RFM12_PHY_getStatus(void);

#endif /*RFM12_H_*/
