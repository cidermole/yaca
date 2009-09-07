#ifndef _MCP2515_H_
#define _MCP2515_H_

/**
  \file mcp2515.h
  \brief Global defines and function headers for MCP2515 CAN chip driver

  You need to take care of interrupt handling yourself, the functions assume the SPI module is free!
  See MCP2515 datasheet for details on implementation.

*/


#include <avr/io.h>
#include <inttypes.h>


/****************  SPI PIN DEFINITIONS  ***************
 *
 * Except for CS (Chip Select), these pins can't really
 * be changed because hardware SPI requires these pins.
 *
 */
#define DDR_CS   DDRB
#define PORT_CS  PORTB
#define P_CS     2

#define DDR_SPI  DDRB
#define PORT_SPI PORTB
#define P_MISO   4
#define P_MOSI   3
#define P_SCK    5


// prescaler for clock output of MCP2515, see datasheet p. 58. Be sure to include CLKEN.
#define CLOCK_PRESCALER_MAGIC 0x07


#include "mcp2515io.h"



typedef struct {
  uint8_t reg;
  uint8_t val;
} mcp2515_init_t;

typedef union {
  uint16_t asint;
  mcp2515_init_t asinit;
} mcp2515_init_convert_t;


#define HIGH(a) ((uint8_t)(a >> 8))
#define LOW(a) ((uint8_t)(a))


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

	\def _spi_getc()
	\brief Read a byte from SPI.

	This is accomplished by calling _spi_putc() with a dummy byte written.

*/
#define _spi_getc() _spi_putc(0xFF)

/**

	\brief Write a byte to an MCP2515 register.

	\param[in] address MCP2515 register address
	\param[in] data Data byte to be written

*/
void mcp2515_wrreg(uint8_t address, uint8_t data);

/**

	\brief Read MCP2515 status

	Issue status read command to MCP2515, reading status flags.

	\param[in] cmd Status command, may be MCP2515_CMD_RX_STATUS or MCP2515_CMD_READ_STATUS

	\return status byte

*/
uint8_t mcp2515_rdstatus(uint8_t cmd);

/**

	Initialize the ATmega SPI interface.

*/
void _spi_init();


/**

	\brief Initialize the MCP2515 driver.

	The SPI interface will be initialized too. The global interrupt flag needs to be enabled separately.

*/
void mcp2515_init();

/**

	\brief Write and read a byte on SPI.

	If you want to read a byte, write a dummy byte e.g. 0xFF.

	\sa _spi_getc()

	\param[in] data Data byte to be written.
	\return Data byte read from SPI.

*/
uint8_t _spi_putc(uint8_t data);

/**

	\brief Set external interrupt pin.
	
	\param[in] val if non-zero, enables interrupt; else, disables it

*/
void mcp2515_set_int(uint8_t val);


#endif /* _MCP2515_H_ */

