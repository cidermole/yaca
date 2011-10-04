#ifndef SPI_H
#define SPI_H

#include <inttypes.h>

#if defined(__AVR_ATmega8__)
#include "spi_atmega8.h"
#elif defined(__AVR_ATmega1284P__)
#include "spi_atmega1284p.h"
#else
#error "No SPI implementation for the current MCU, at the very least for AVR, the SPI pins need to be defined."
#endif

/**

	Initialize the ATmega SPI interface.

*/
void _spi_init();

/**

	\def _spi_start()
	\brief Activate Chip Select output

	To write/read a register of MCP2515, one needs to pull the CS low, transmit
	and receive the necessary bytes, and let go of CS (-> high).

	\sa _spi_stop()

*/
#define _spi_start() PORT_CS &= ~(1 << P_CS)

/**

	\def _spi_stop()
	\brief Deactivate Chip Select output

	To write/read a register of MCP2515, one needs to pull the CS low, transmit
	and receive the necessary bytes, and let go of CS (-> high).

	\sa _spi_start()

*/
#define _spi_stop()  PORT_CS |= (1 << P_CS)

/**

	\brief Write and read a byte on SPI.

	If you want to read a byte, write a dummy byte e.g. 0xFF.

	\sa _spi_getc()

	\param[in] data Data byte to be written.
	\return Data byte read from SPI.

*/
uint8_t _spi_putc(uint8_t data);

/**

	\def _spi_getc()
	\brief Read a byte from SPI.

	This is accomplished by calling _spi_putc() with a dummy byte written.

*/
#define _spi_getc() _spi_putc(0xFF)


#endif /* SPI_H */

