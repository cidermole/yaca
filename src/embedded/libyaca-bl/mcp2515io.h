#define MCP2515_CMD_RESET       0xC0
#define MCP2515_CMD_READ        0x03
#define MCP2515_CMD_READ_RX     0x90
#define MCP2515_CMD_WRITE       0x02
#define MCP2515_CMD_WRITE_TX    0x40
#define MCP2515_CMD_RTS         0x80
#define MCP2515_CMD_READ_STATUS 0xA0
#define MCP2515_CMD_RX_STATUS   0xB0
#define MCP2515_CMD_BIT_MODIFY  0x05




// b means bit modify command will work

#define RXF0SIDH   0x00
#define RXF0SIDL   0x01
#define RXF0EID8   0x02
#define RXF0EID0   0x03
#define RXF1SIDH   0x04
#define RXF1SIDL   0x05
#define RXF1EID8   0x06
#define RXF1EID0   0x07
#define RXF2SIDH   0x08
#define RXF2SIDL   0x09
#define RXF2EID8   0x0A
#define RXF2EID0   0x0B
#define BFPCTRL    0x0C // b
#define TXRTSCTRL  0x0D // b
#define CANSTAT    0x0E
#define CANCTRL    0x0F // b


#define RXF3SIDH   0x10
#define RXF3SIDL   0x11
#define RXF3EID8   0x12
#define RXF3EID0   0x13
#define RXF4SIDH   0x14
#define RXF4SIDL   0x15
#define RXF4EID8   0x16
#define RXF4EID0   0x17
#define RXF5SIDH   0x18
#define RXF5SIDL   0x19
#define RXF5EID8   0x1A
#define RXF5EID0   0x1B
#define TEC        0x1C
#define REC        0x1D


#define RXM0SIDH   0x20
#define RXM0SIDL   0x21
#define RXM0EID8   0x22
#define RXM0EID0   0x23
#define RXM1SIDH   0x24
#define RXM1SIDL   0x25
#define RXM1EID8   0x26
#define RXM1EID0   0x27
#define CNF3       0x28 // b
#define CNF2       0x29 // b
#define CNF1       0x2A // b
#define CANINTE    0x2B // b
#define CANINTF    0x2C // b
#define EFLG       0x2D // b


#define TXB0CTRL   0x30 // b
#define TXB0SIDH   0x31
#define TXB0SIDL   0x32
#define TXB0EID8   0x33
#define TXB0EID0   0x34
#define TXB0DLC    0x35
#define TXB0D0     0x36
#define TXB0D1     0x37
#define TXB0D2     0x38
#define TXB0D3     0x39
#define TXB0D4     0x3A
#define TXB0D5     0x3B
#define TXB0D6     0x3C
#define TXB0D7     0x3D


#define TXB1CTRL   0x40 // b
#define TXB1SIDH   0x41
#define TXB1SIDL   0x42
#define TXB1EID8   0x43
#define TXB1EID0   0x44
#define TXB1DLC    0x45
#define TXB1D0     0x46
#define TXB1D1     0x47
#define TXB1D2     0x48
#define TXB1D3     0x49
#define TXB1D4     0x4A
#define TXB1D5     0x4B
#define TXB1D6     0x4C
#define TXB1D7     0x4D


#define TXB2CTRL   0x50 // b
#define TXB2SIDH   0x51
#define TXB2SIDL   0x52
#define TXB2EID8   0x53
#define TXB2EID0   0x54
#define TXB2DLC    0x55
#define TXB2D0     0x56
#define TXB2D1     0x57
#define TXB2D2     0x58
#define TXB2D3     0x59
#define TXB2D4     0x5A
#define TXB2D5     0x5B
#define TXB2D6     0x5C
#define TXB2D7     0x5D


#define RXB0CTRL   0x60 // b
#define RXB0SIDH   0x61
#define RXB0SIDL   0x62
#define RXB0EID8   0x63
#define RXB0EID0   0x64
#define RXB0DLC    0x65
#define RXB0D0     0x66
#define RXB0D1     0x67
#define RXB0D2     0x68
#define RXB0D3     0x69
#define RXB0D4     0x6A
#define RXB0D5     0x6B
#define RXB0D6     0x6C
#define RXB0D7     0x6D


#define RXB1CTRL   0x70 // b
#define RXB1SIDH   0x71
#define RXB1SIDL   0x72
#define RXB1EID8   0x73
#define RXB1EID0   0x74
#define RXB1DLC    0x75
#define RXB1D0     0x76
#define RXB1D1     0x77
#define RXB1D2     0x78
#define RXB1D3     0x79
#define RXB1D4     0x7A
#define RXB1D5     0x7B
#define RXB1D6     0x7C
#define RXB1D7     0x7D


#define BTLMODE    7
#define SAM        6
#define PHSEG12    5
#define PHSEG11    4
#define PHSEG10    3
#define PRSEG2     2
#define PRSEG1     1
#define PRSEG0     0

#define SOF        7
#define WAKFIL     6
#define PHSEG22    2
#define PHSEG21    1
#define PHSEG20    0

#define MERRE      7
#define WAKIE      6
#define ERRIE      5
#define TX2IE      4
#define TX1IE      3
#define TX0IE      2
#define RX1IE      1
#define RX0IE      0

#define MERRF      7
#define WAKIF      6
#define ERRIF      5
#define TX2IF      4
#define TX1IF      3
#define TX0IF      2
#define RX1IF      1
#define RX0IF      0

#define RX1OVR     7
#define RX0OVR     6
#define TXBO       5
#define TXEP       4
#define RXEP       3
#define TXWAR      2
#define RXWAR      1
#define EWARN      0

#define ABTF       6
#define MLOA       5
#define TXERR      4
#define TXREQ      3
#define TXP1       1
#define TXP0       0

#define RXM1       6
#define RXM0       5
#define RXTR       3
#define BUKT       1
#define FILHIT0    0

#define FILHIT2    2
#define FILHIT1    1
#define FILHIT0    0

#define IDE        3
#define EXIDE      3
#define RTR        6
