#!/usr/bin/env python

import serial
import datetime
import time

ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=10)

def read_count():
	ser.write('\x1B') # ESC
	time.sleep(0.1)
	ser.write('C')

	#print('reading...')
	ser.readline()
	ser.readline()
	ser.readline()
	#print('>%s<' % ser.readline())
	#print('>%s<' % ser.readline())
	#print('>%s<' % ser.readline())

	count = ser.readline()

	ser.readline()
	ser.readline()
	#print('>%s<' % ser.readline())
	#print('>%s<' % ser.readline())
	#print('reading done.')

	return count[21:].rstrip('\r\n')

while True:
	curdt = datetime.datetime.fromtimestamp(time.time())
	time.sleep(60 - curdt.second)
	curdt += datetime.timedelta(minutes=1)
	# once per hour
	if curdt.minute == 0:
		count = read_count()
		print('%04d-%02d-%02d %02d:%02d;%s' % (curdt.year, curdt.month, curdt.day, curdt.hour, curdt.minute, count))
