#ifndef RADIO_H
#define RADIO_H

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
	} flags;
	uint8_t length;
	uint8_t data[8];
	uint16_t crc16; // crc16 includes whole struct + radio id
} STRUCT_PACKED RadioMessage;

typedef enum {
    PENDING = 2, ///< waiting for transmission queue, acknowledge or timeout
    SUCCESS = 1, ///< frame successfully sent
    FAILURE = 0 ///< error transmitting frame
} tstatus;

typedef enum {
	ST_IDLE,
	ST_TX,
	ST_RX
} radio_state_t;

void radio_init(uint8_t radio_id_node);
uint8_t radio_poll_receive();
void radio_receive(RadioMessage *msg);
tstatus radio_transmit(uint8_t radio_id_target, RadioMessage *msg);
uint16_t radio_crc(uint8_t radio_id, RadioMessage *msg);

#endif /* RADIO_H */

