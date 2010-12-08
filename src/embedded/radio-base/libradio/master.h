#ifndef MASTER_H
#define MASTER_H

#include <stdint.h>

#define SLOT_LENGTH 150 // slot length in ms

// slot assignments in EEPROM
typedef struct {
	uint8_t radio_id;
	uint16_t slot;
} slot_assign_t;

typedef struct {
	uint8_t rx_fc, tx_fc; // framecounter
	uint8_t tx_state[16], tx_state_old[16]; // tx_state_old for ACK retransmission
	uint8_t rx_state[16];
} slot_t;

typedef enum {
	NORMAL,
	RETRY
} retr_e;

slot_t *find_slot(uint8_t radio_id);
tstatus _master_radio_transmit(uint8_t radio_id, RadioMessage *msg, uint8_t *tx_state);
void _send_ack(uint8_t radio_id, slot_t *slot, retr_e retr);

#endif /* MASTER_H */

