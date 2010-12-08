#ifndef RADIO_H
#define RADIO_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

#if defined(__AVR__)
#define STRUCT_PACKED
#else
#define STRUCT_PACKED __attribute__((packed))
#endif

#ifndef NULL
#define NULL 0
#endif

typedef struct {
	uint8_t info; // not actually transmitted
	uint8_t fc; // framecounter (not crypted)
	uint32_t can_id;
	struct {
		uint8_t rtr: 1;
		uint8_t can: 1; // 1: CAN message, 0: general radio message
		uint8_t ack: 1;
	} flags;
	uint8_t length;
	uint8_t data[8];
	uint16_t crc16; // crc16 includes whole struct + radio id + framecounter and must be the last item
} STRUCT_PACKED RadioMessage;

#ifndef _LIBYACA_H_ // yaca.h defines the same enum types

typedef enum {
    PENDING = 2, ///< waiting for transmission queue, acknowledge or timeout
    SUCCESS = 1, ///< frame successfully sent
    FAILURE = 0 ///< error transmitting frame
} tstatus;

#endif

typedef enum {
	ST_IDLE,
	ST_TX,
	ST_RX
} radio_state_t;

void protocol_dispatch(uint8_t radio_id, RadioMessage *msg);
uint16_t radio_crc(uint8_t radio_id, RadioMessage *msg);

// for master: call srandom() with a good seed
void radio_init(uint8_t radio_id_node);

uint8_t radio_poll_receive();
void radio_receive(RadioMessage *msg);
tstatus radio_transmit(uint8_t radio_id_target, RadioMessage *msg);
void radio_slave_resync(); // call directly after radio_init()
tstatus radio_retransmit(uint8_t radio_id, RadioMessage *msg);

#ifdef __cplusplus
}
#endif

#endif /* RADIO_H */

