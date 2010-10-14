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
#ifndef RFM12_STACK_H_
#define RFM12_STACK_H_

#include <stdint.h>
#include <stdbool.h>

/******************** Layer 3: Network   **********************/

// End of data
#define RFM12_L3_EOD	-1

typedef uint8_t	RFM12_ProtocolID_t;

/**
 * Callback when a byte was received.
 * @param data	byte that was received
 */
typedef void (RFM12_L3_rxCallback)(int16_t data);

/**
 * Callback when next byte can be sent.
 * @return the next byte to send
 */
typedef int16_t (RFM12_L3_txCallback)(void);


/******************** Layer 2: Data Link **********************/

/*********** LLC ***********/

extern RFM12_L3_rxCallback *RFM12_LLC_protocol_rxCallback;
extern RFM12_L3_txCallback *RFM12_LLC_protocol_txCallback;

/**
 * Decode next byte.
 * @param data the received byte
 * @return true if packet end detected
 */
extern inline bool RFM12_LLC_receiveCallback(uint8_t data);

/**
 * Get next byte to transmit.
 * @return next byte to transmit
 */
extern inline uint8_t RFM12_LLC_transmitCallback(void);

/*********** MAC ***********/

// TODO: delay control, collision avoidance

typedef enum {
	RFM12_MAC_INIT,
	RFM12_MAC_IDLE,
	RFM12_MAC_RX,
	RFM12_MAC_RX_WAIT,
	RFM12_MAC_TX,
	RFM12_MAC_TX_WAIT
} RFM12_MAC_State_t;

/**
 * Decode next byte.
 * @param data the received byte
 * @return true if packet end detected
 */
extern inline bool RFM12_MAC_receiveCallback(uint8_t data);

/**
 * Get next byte to transmit.
 * @return next byte to transmit
 */
extern inline uint8_t RFM12_MAC_transmitCallback(void);

/******************** Layer 1: Physical ***********************/

typedef enum {
	RFM12_LISTEN,
	RFM12_RX,
	RFM12_TX
} RFM12_PHY_State_t;


#endif /*RFM12_STACK_H_*/
