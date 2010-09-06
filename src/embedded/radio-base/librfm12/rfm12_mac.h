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
#ifndef RFM12_MAC_H_
#define RFM12_MAC_H_

#include <stdint.h>
#include <stdbool.h>
#include "rfm12_stack.h"

/********************************************************************************
 * CONFIGURATION
 ********************************************************************************/

#define RFM12_MAC_MAX_DATA_LENGTH	64
#define RFM12_MAC_EOF				0xAA

#define RFM12_MAC_TIMER_COMPARE_vect	TIMER0_COMP_vect

#define RFM12_MAC_CTRL_BW		RxBW200
#define RFM12_MAC_CTRL_DEV		TxBW75
#define RFM12_MAC_CTRL_BAUD		9600

#define RFM12_MAC_BURST_BW		RxBW270
#define RFM12_MAC_BURST_DEV		TxBW150

#define RFM12_MAC_TX_WAIT_TIME	500


/********************************************************************************
 * INTERFACE
 ********************************************************************************/

/**
 *******************************************************************************
 * Set the active channel.
 * @param channel	the new channel (0-15)
 *******************************************************************************
 */
void RFM12_MAC_setChannel(uint8_t channel);

/**
 *******************************************************************************
 * Check if the RFM12 is currently receiving a frame.
 * @return true if active
 *******************************************************************************
 */
bool RFM12_MAC_isReceiving(void);

/**
 *******************************************************************************
 * Check if the media is busy.
 * @return true if RX or TX active or signal strength over threshold
 *******************************************************************************
 */
bool RFM12_MAC_mediaBusy(void);

/**
 *******************************************************************************
 * Start a transmission over the control channel.
 * Blocks until media becomes free.
 *******************************************************************************
 */
void RFM12_MAC_startCtrlTransmission(void);

/**
 *******************************************************************************
 * Inform the MAC that a transmission has finished.
 *******************************************************************************
 */
void RFM12_MAC_endCtrlTransmission(void);


#endif /*RFM12_MAC_H_*/
