#ifndef _LIBYACA_H_
#define _LIBYACA_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
  \file yaca.h
  \brief yaca library

  Functions for communication between nodes; the main program doesn't need
  to know how this happens but relies on correct transmission or error notification.
  libyaca is linked to the libyaca-bl trampolines in bootloader, so the communication
  routines are present only one time in flash.
  The main usage of this header is to send messages (via yc_send() ) or remote transmission
  requests (via yc_rtr() ). Before calling these, the target CAN-ID needs
  to be set with yc_prepare(canid). Example:
  \code
  yc_prepare(0x12345678);
  yc_send(Switch, SetLight(1));
  \endcode
  This invokes the SetLight message on a remote Switch type node if it is listening
  for CAN-ID 0x12345678.
  yc_send() and yc_rtr() are just macros for calling local dispatcher functions
  which send the appropriate CAN frame with the corresponding params; see the
  yaca "compiler" source for details on how this is implemented.

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
	uint8_t info; ///< was frame_id, local id of frame. MUST BE 0 prior to transmission
	uint32_t id;
	uint8_t rtr; ///< Whether this is a remote transmission request
	uint8_t length;
	uint8_t data[8];
} Message;

/**
	\brief Function pointer table structure

	The function pointer table is in flash memory.
*/
typedef struct {
	uint8_t packstyle;
	void *fp;
} fpt_t;

/**
	\brief Performs necessary initialization of communication modules

	\return zero if something went wrong
*/
uint8_t yc_init();

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

  \return Zero if the operation fails
  \sa yc_poll_receive()

*/
uint8_t yc_receive(Message* f);

/**

  \brief Checks for communication errors

  \return non-zero error code if something went wrong somewhere
  \sa error codes in implementation

*/
uint8_t yc_get_error();

/**

  \brief Dispatch a message according to CAN-ID tables

  \code
    typedef struct {
      uint8_t packstyle;
      void* fp;
    } midt;

    midt mid[] PROGMEM = { { 0x13, remote }, { 0x13, remote } }; // this should be generated by the yaca "compiler"
    uint8_t midCount PROGMEM = 2;
    // ...
    yc_dispatch(&msg, (uint8_t*)0, (void**) mid, (uint8_t*)&midCount); // will call the local RPC response function if a matching CAN-ID is found
  \endcode

  \param[in] m pointer to message to be dispatched
  \param[in] eep_idtable EEPROM address of CAN-ID table
  \param[in] flash_fps Flash address of function pointer table
  \param[in] flash_count Flash address of uint8_t field containing the function pointer count

*/
void yc_dispatch(Message* m, uint8_t* eep_idtable, void** flash_fps, uint8_t* flash_count);

/**

  \brief Pack function registers and send CAN message of an RPC call

  This function is called by assembler code generated by the yaca "compiler". There is no use for it manually.

*/
void _pack_n_go();

/**

  \brief Prepare for an RPC call by setting remote CAN-ID

  This is a convenience function for preloading a global CAN-ID variable, which is used in all outgoing RPC calls.

*/
void yc_prepare(uint32_t canid);

/**

  \brief Prepare for an RPC call by setting remote CAN-ID

  This is a convenience function for preloading a global CAN-ID variable, which is used in all outgoing RPC calls, from an address in EEPROM.

  \param[in] pcanid EEPROM address of a CAN-ID

*/
void yc_prepare_ee(uint8_t *pcanid);

/**

  \brief Unload communication devices

  This one is used in the bootloader while flashing a page (takes 4 ms, while CAN frames can arrive in 0,5 ms intervals).

*/
void yc_close();

#ifdef __cplusplus
}
#endif

#endif /* _LIBYACA_H_ */

