#!/usr/bin/env python

import MySQLdb
import socket
import struct
from datetime import datetime

conn = MySQLdb.connect(host="127.0.0.1", user="yaca", passwd="UdxYzj34", db="yaca")
cursor = conn.cursor()

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('192.168.1.1', 1222))

CAR_COUNT_CANID = 406

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

def fdate(d):
	return '%04d-%02d-%02d %02d:%02d:%02d' % (d.year, d.month, d.day, d.hour, d.minute, d.second)

while True:
	msg = get_message(sock)
	if msg[M_ID] == CAR_COUNT_CANID and msg[M_RTR] == 0:
		count = msg[M_DATA] * (256**3) + msg[M_DATA + 1] * (256**2) + msg[M_DATA + 2] * (256**1) + msg[M_DATA + 3]
		stamp = datetime.today()
		cursor.execute('insert into cars values (\'%s\', %d, %d)' % (fdate(stamp), stamp.microsecond / 1000, count))

sock.close()
cursor.close()
conn.close()

