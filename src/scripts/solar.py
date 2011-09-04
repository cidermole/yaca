#!/usr/bin/env python2

import socket
import struct

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('192.168.1.1', 1222))

JOULESTATUS_CANID = 407
POWERSTATUS_CANID = 408

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
	if msg[M_ID] == JOULESTATUS_CANID and msg[M_RTR] == 0:
		joule_battery = msg[M_DATA] * (256**3) + msg[M_DATA + 1] * (256**2) + msg[M_DATA + 2] * (256**1) + msg[M_DATA + 3]
		joule_solar = msg[M_DATA + 4] * (256**3) + msg[M_DATA + 5] * (256**2) + msg[M_DATA + 6] * (256**1) + msg[M_DATA + 7]
		print('battery: %6d J (%6.3lf %%), solar: %6d J' % (joule_battery, float(joule_battery) / (12.0 * 7 * 3600 / 100), joule_solar))
	if msg[M_ID] == POWERSTATUS_CANID and msg[M_RTR] == 0:
		# vbat, ibat, isol
		vbat = msg[M_DATA] * (256**1) + msg[M_DATA + 1]
		ibat = msg[M_DATA + 2] * (256**1) + msg[M_DATA + 3]
		if ibat >= 2**15:
			ibat = 2**16 - ibat
		isol = msg[M_DATA + 4] * (256**1) + msg[M_DATA + 5]
		print('                                                     Vbat: %5.3lf V, Ibat: %4.3lf A, Isol: %4.3lf A, Pbat: %5.3lf W, Psol: %5.3lf W' % (0.015 * vbat, 0.0065 * ibat, 0.0065 * isol, 0.015 * vbat * 0.0065 * ibat, 0.015 * vbat * 0.0065 * isol))

sock.close()

