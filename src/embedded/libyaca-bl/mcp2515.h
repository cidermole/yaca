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


// prescaler for clock output of MCP2515, see datasheet p. 58. Be sure to include CLKEN.
#define MCP2515_CLKOUT_2MHZ 0x07
#define MCP2515_CLKOUT_16MHZ 0x04
//#define CLOCK_PRESCALER_MAGIC
// CLOCK_PRESCALER_MAGIC is defined by compiler option


#include "mcp2515io.h"



typedef struct {
  uint8_t reg;
  uint8_t val;
} mcp2515_init_t;

typedef union {
  uint16_t asint;
  mcp2515_init_t asinit;
} mcp2515_init_convert_t;


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

	\brief Initialize the MCP2515 driver.

	The SPI interface will be initialized too. The global interrupt flag needs to be enabled separately.

*/
void mcp2515_init();

/**

	\brief Set external interrupt pin.
	
	\param[in] val if non-zero, enables interrupt; else, disables it

*/
void mcp2515_set_int(uint8_t val);


#endif /* _MCP2515_H_ */

