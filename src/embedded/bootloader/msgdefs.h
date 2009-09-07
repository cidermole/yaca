#ifndef _MSGDEFS_H_
#define _MSGDEFS_H_

// Temp-ID db0 values
#define TID_REVOCATE	0x01	// Msg#02: Temp-ID revocation
#define TID_CIDW_BLOCK	0x02	// Msg#12: CANid data block write [1: offset (1), 2: count (1), 3: data]
#define TID_CIDR_SINGLE	0x03	// Msg#13: Single CANid read [1: mi (1), 2: ci (1)] // message index, canid sub-index
#define TID_CIDT_SINGLE	0x04	// Msg#14: CANid reply [1: CANid (4)]
#define TID_CIDR_BLOCK	0x05	// Msg#15: CANid data block read [1: offset (1)]
#define TID_CIDT_BLOCK	0x06	// Msg#16: CANid data block reply [1: data]
#define TID_CIDWDONE	0x07	// Msg#17: CANid data block write done
#define TID_SET_MID		0x08	// Msg#18: Change Master-ID [1: new master id (4)]
#define TID_BLD_ENTER	0x09	// Msg#19: Enter bootloader
// BLD update: no implement
#define TID_BLD_PAGESEL	0x0B	// Msg#21: Bootloader page select [1: page (2)]
#define TID_BLD_DATA	0x0C	// Msg#22: Bootloader page data [1: data]			+increment ptr!
#define TID_BLD_PGDONE	0x0D	// Msg#23: Bootloader page done
#define TID_BLD_BOOT	0x0E	// Msg#24: Boot application
#define TID_BLD_EE_WR	0x0F


// TODO: Docu:: enter Master-ID, 0xFE, 0x03 error as "unknown TID/MID msg."

#endif /* _MSGDEFS_H_ */

