#ifndef _YACA_BL_H_
#define _YACA_BL_H_

#ifdef __cplusplus
extern "C" {
#endif


#define YCERR_CAN_STD_FRAME		(1 << 0) // We have received a CAN frame with standard (11-bit) ID. This bus is extended-ID only!
#define YCERR_MCU_RX_OVERRUN	(1 << 1) // Our 1-message "buffer" has overflowed!
#define YCERR_BLD_FSM			(1 << 2) // The bootloader FSM reached an invalid state
#define YCERR_CRC				(1 << 3) // The bootloader has detected a CRC error in the application
#define YCERR_PAGE_OVERRUN		(1 << 4) // The bootloader reached an invalid page
#define YCERR_MULTI				(1 << 7) // Multiple errors (EEPROM error flags already used)


/**
  \file yaca-bl.h
  \brief Bootloader yaca library

  See cwrap.h in libyaca-bl for details, some comments below are copied from there.
  Functions for communication between nodes; the main program doesn't need
  to know how this happens but relies on correct transmission or error notification.

*/

#include <inttypes.h>

/**

  \brief Transmission status

  \sa yc_transmit(), yc_poll_transmit()

*/
typedef enum {
	PENDING = 2, ///< waiting for transmission queue, acknowledge or timeout
	SUCCESS = 1, ///< frame successfully sent
	FAILURE = 0 ///< error transmitting frame
} tstatus;

/**
	\brief Communication message frame

	Note that this structure is hardcoded in assembler (asms.S).

	\sa yc_transmit(), yc_receive()
*/
typedef struct {
	uint8_t info; ///< was frame_id, local id of frame
	uint32_t id;
	uint8_t rtr; ///< Whether this is a remote transmission request
	uint8_t length;
	uint8_t data[8];
} Message;

/**
	\brief Performs necessary initialization of communication modules
*/
void yc_init();

/**

  \brief Transmits a frame and sets frame id

  This function shall not block waiting for successful transmission or acknowledge,
  but may wait until the whole frame is written into the transmit device buffer - e.g.
  await the end of an SPI transmission.
  May return PENDING if the transmission buffer is full, see yc_poll_transmit()

  \param[in] f pointer to the frame to be transmitted

  \return PENDING, SUCCESS or FAILURE
  \sa tstatus, yc_poll_transmit()

*/
tstatus yc_transmit(Message* f);

/**

  \brief Poll transmission status

  Needs the frame id set - call yc_transmit() first. Might also try (re)transmission -
  watch out, this needs to be called then, otherwise transmission is tried only once.

  \param[in] f pointer to the frame to be polled (needs to have frame id set)

  \return PENDING, SUCCESS or FAILURE
  \sa yc_transmit()

*/
tstatus yc_poll_transmit(Message* f);

/**

  \brief Poll receive buffer

  Checks if a frame has been received

  \return Non-zero if a frame is in the receive buffer
  \sa yc_receive()

*/
uint8_t yc_poll_receive();

/**

  \brief Read a frame from the receive buffer

  \param[out] f pointer to the target frame structure

  \sa yc_poll_receive()

*/
void yc_receive(Message* f);

/**

  \brief Checks for communication errors

  \return non-zero error code if something went wrong somewhere
  \sa error codes in implementation

*/
uint8_t yc_get_error();

/**

  \brief Unload communication devices

  This one is used in the bootloader while flashing a page (takes 4 ms, while CAN frames can arrive in 0,5 ms intervals).

*/
void yc_close();


#ifdef __cplusplus
}
#endif

#endif /* _YACA_BL_H_ */

