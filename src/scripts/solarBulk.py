#!/usr/bin/env python2

import socket
import struct
import sys
import dateutil.parser

#sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#sock.connect(('192.168.1.1', 1222))

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


# data offset, i.e. where hex starts in line
DATA_OFFSET = 38

def unhex(s):
    hexchars = '0123456789ABCDEF'
    hexchars2 = '0123456789abcdef'
    hexsum = 0
    for i in range(0, len(s)):
        digit = hexchars.find(s[i])
        if digit == -1:
            digit = hexchars2.find(s[i])
            if digit == -1:
                sys.stderr.write('error unhexing %s\n' % s)
        hexsum = hexsum * 16 + digit
    return hexsum



# '2012-05-06 22:25:37.578 [  408]   (8) 0336000F00000026'

def get_message(line):
	global DATA_OFFSET
	linedata = line[DATA_OFFSET:].strip()
	s = b''
	for i in range(0, len(linedata), 2):
		s += bytes(chr(unhex(linedata[i:i+2])))
	data = struct.unpack('<'+'B'*(len(linedata)/2), s)
	rtr = (line.split()[4] == 'R')
	l = line.split()[4].strip('()') if not rtr else line.split()[5].strip('()')
	i = line.split()[3].strip('[]')
	date = line.split()[0]
	time = line.split()[1]
	info = [date, time, dateutil.parser.parse(date + ' ' + time)]
	m = [info, int(i), 1 if rtr else 0, int(l)] + list(data)
	return m

min_prev=-1
na1 = False
na2 = False
for line in sys.stdin:
	#if len(line) < 54:
	if len(line) < DATA_OFFSET:
		# skip broken lines
		sys.stderr.write('broken line: %s' % line)
		continue
	try:
		msg = get_message(line)
	except ValueError:
		sys.stderr.write('unhex error in line: %s' % line)
		continue
	if msg[M_INFO][2].minute == min_prev:
		if na1 and na2:
			na1 = False
			na2 = False
			min_prev = msg[M_INFO][2].minute
			continue
	else:
		pass
	if not na1 and not na2:
		#print('%s %s' % (msg[M_INFO][0], msg[M_INFO][1]))
		pass
	if msg[M_ID] == JOULESTATUS_CANID and msg[M_RTR] == 0:
		joule_battery = msg[M_DATA] * (256**3) + msg[M_DATA + 1] * (256**2) + msg[M_DATA + 2] * (256**1) + msg[M_DATA + 3]
		joule_solar = msg[M_DATA + 4] * (256**3) + msg[M_DATA + 5] * (256**2) + msg[M_DATA + 6] * (256**1) + msg[M_DATA + 7]
		#print('battery: %6d J (%6.3lf %%), solar: %6d J' % (joule_battery, float(joule_battery) / (12.0 * 7 * 3600 / 100), joule_solar))
		na1 = True
	if msg[M_ID] == POWERSTATUS_CANID and msg[M_RTR] == 0:
		# vbat, ibat, isol
		vbat = msg[M_DATA] * (256**1) + msg[M_DATA + 1]
		ibat = msg[M_DATA + 2] * (256**1) + msg[M_DATA + 3]
		if ibat >= 2**15:
			ibat = -(2**16 - ibat)
		isol = msg[M_DATA + 4] * (256**1) + msg[M_DATA + 5]
		#print('                                                     Vbat: %5.3lf V, Ibat: %4.3lf A, Isol: %4.3lf A, Pbat: %5.3lf W, Psol: %5.3lf W' % (0.015 * vbat, 0.0065 * ibat, 0.0065 * isol, 0.015 * vbat * 0.0065 * ibat, 0.015 * vbat * 0.0065 * isol))
		print('%s %s  Vbat: %5.3lf V, Ibat: %4.3lf A, Isol: %4.3lf A, Pbat: %5.3lf W, Psol: %5.3lf W' % (msg[M_INFO][0], msg[M_INFO][1], 0.015 * vbat, 0.0065 * ibat, 0.0065 * isol, 0.015 * vbat * 0.0065 * ibat, 0.015 * vbat * 0.0065 * isol))
		na2 = True
	min_prev = msg[M_INFO][2].minute

#sock.close()

