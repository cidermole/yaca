#ifndef SPI_ATMEGA1284P_H
#define SPI_ATMEGA1284P_H

/****************  SPI PIN DEFINITIONS  ***************
 *
 * Except for CS (Chip Select), these pins can't really
 * be changed because hardware SPI requires these pins.
 *
 */
#define DDR_CS   DDRB
#define PORT_CS  PORTB
#define P_CS     4

#define DDR_SPI  DDRB
#define PORT_SPI PORTB
#define P_MISO   6
#define P_MOSI   5
#define P_SCK    7

#endif /* SPI_ATMEGA1284P_H */

