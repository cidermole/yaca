#ifndef SPI_ATMEGA8_H
#define SPI_ATMEGA8_H

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

#endif /* SPI_ATMEGA8_H */

