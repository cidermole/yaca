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
#ifndef RFM12_LLC_H_
#define RFM12_LLC_H_

#include <stdint.h>
#include <stdbool.h>
#include "rfm12_stack.h"

#define RFM12_LLC_RX_BUFFER_SIZE	2

typedef enum RFM12_FrameType {
	TYPE_MANAGEMENT,
	TYPE_SERIAL
} RFM12_FrameType_t;

typedef struct RFM12_FrameHeader {
	uint8_t receiver;
	uint8_t sender;
	uint8_t requireACK:1;
	uint8_t isACK:1;
	uint8_t protID:4;
} RFM12_LLC_FrameHeader_t;


/**
 *******************************************************************************
 * Register a handler for a protocol ID.
 * @param typeID	the ID of the protocol
 * @param proto		the protocol handler
 *******************************************************************************
 */
void RFM12_LLC_registerType(RFM12_ProtocolID_t typeID, RFM12_L3_Protocol_t proto);



/**
 *******************************************************************************
 * Sends a frame over the radio.
 * @param frame	the frame to send
 * @return ACK, NACK, or TIMEOUT
 *******************************************************************************
 */
uint8_t RFM12_LLC_sendFrame(RFM12_ProtocolID_t proto, uint8_t receiver, bool requireACK);

#endif /*RFM12_LLC_H_*/
