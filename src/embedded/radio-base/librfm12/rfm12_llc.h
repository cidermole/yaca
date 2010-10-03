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

/**
 *******************************************************************************
 * Register a handler for a protocol ID.
 *******************************************************************************
 */
void RFM12_LLC_registerType(RFM12_L3_rxCallback *rxCallback, RFM12_L3_txCallback *txCallback);



/**
 *******************************************************************************
 * Sends a frame over the radio.
 * @param frame	the frame to send
 * @return ACK, NACK, or TIMEOUT
 *******************************************************************************
 */
uint8_t RFM12_LLC_sendFrame();

#endif /*RFM12_LLC_H_*/
