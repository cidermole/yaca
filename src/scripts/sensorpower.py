#!/usr/bin/env python2

import socket
import struct

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('192.168.1.1', 1222))

TEMP_CANID = 405

#   struct Message {
#           uint8_t info;
#           uint32_t id;
#           uint8_t rtr;
#           uint8_t length;
#           uint8_t data[8];
#   } __attribute__((__packed__));

M_INFO = 0
M_ID   = 1
M_RTR  = 2
M_LEN  = 3
M_DATA = 4

def get_message(s):
	str = ''
	while len(str) < 15:
		str += s.recv(15 - len(str))
	return struct.unpack('<BIBBBBBBBBBB', str)

while True:
	msg = get_message(sock)
	if msg[M_ID] == TEMP_CANID and msg[M_RTR] == 0:
		temp = msg[M_DATA+0] * (256**1) + msg[M_DATA+1]
		if temp >= 2**15:
			temp = -(2**16 - temp)
		temp /= 10.0
		millivolt = msg[M_DATA+2] * (256**1) + msg[M_DATA+3]
		print('temp: %.1lf deg C voltage: %d millivolt (%.0lf mV/cell)' % (temp, millivolt, millivolt / 3.0))

sock.close()

