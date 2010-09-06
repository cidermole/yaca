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
#ifndef HAMMING_H_
#define HAMMING_H_

#include <avr/pgmspace.h>

extern const prog_uint8_t _Hamming_enc[16];
extern const prog_uint8_t _Hamming_dec[256];

#define Hamming_encHigh(data)	pgm_read_byte(&_Hamming_enc[data>>4])
#define Hamming_encLow(data) 	pgm_read_byte(&_Hamming_enc[data&0x0f])

#define Hamming_decHigh(data)	pgm_read_byte(&_Hamming_dec[data])<<4
#define Hamming_decLow(data) 	pgm_read_byte(&_Hamming_dec[data])

#endif /*HAMMING_H_*/
