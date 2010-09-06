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
#ifndef SPI_H_
#define SPI_H_

#if defined(__AVR__)
#include <avr/io.h>

//                                          MOSI        SCK         SS                  MISO
#define SPI_confMaster() 		{ DDRB |= _BV(DDB3) | _BV(DDB5) | _BV(DDB1); DDRB &= ~_BV(DDB4); }

#define SPI_enable()		{ SPCR |= _BV(SPE); }
#define SPI_disable()		{ SPCR &= ~_BV(SPE); }

#define SPI_enableINT()		{ SPCR |= _BV(SPIE); }
#define SPI_disableINT()	{ SPCR &= ~_BV(SPIE); }

#define SPI_master()		{ SPCR |= _BV(MSTR); }
#define SPI_slave()			{ SPCR &= ~_BV(MSTR); }

#define SPI_dordLSB()		{ SPCR |= _BV(DORD); }
#define SPI_dordMSB()		{ SPCR &= ~_BV(DORD); }

#define SPI_cpolIdleHigh()	{ SPCR |= _BV(CPOL); }
#define SPI_cpolIdleLow()	{ SPCR &= ~_BV(CPOL); }

#define SPI_cphaSampFall()	{ SPCR |= _BV(CPHA); }
#define SPI_cphaSampRise()	{ SPCR &= ~_BV(CPHA); }

#define SPI_setFrequency()	{ SPCR &= ~(_BV(SPR1) | _BV(SPR0)); SPSR |= (1 << SPI2X); }	// F_CPU/2 SPI speed

/**
 *******************************************************************************
 * Transfer 8 bits over the SPI interface. (blocking)
 * @param value	the data to send, will contain received data after call
 *******************************************************************************
 */
#define SPI_trans(value, timeout)							\
	timeout = 0;									\
	SPDR = value;											\
	while(!(SPSR & (1<<SPIF)) && ++timeout != 0);			\
	value = SPDR;

#endif

#endif /* SPI_H_ */
