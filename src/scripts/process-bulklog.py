#!/usr/bin/env python

import datetime
import sys

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

# data offset, i.e. where hex starts in line
DATA_OFFSET = 38

##################################################################################################
##################################################################################################

def htime(s, hl):
	reported_time = datetime.datetime(unhex(hl[6:10]), unhex(hl[10:12]), unhex(hl[12:14]), unhex(hl[0:2]), unhex(hl[2:4]), unhex(hl[4:6]))
	flags = unhex(hl[14:16])
	if (flags & 1) != 0:
		tz = 'CEST'
	else:
		tz = 'CET'
	if (flags & 2) != 0:
		bt = ', backup source'
	else:
		bt = ''
	return 'Time::Time %s (%s%s)' % (reported_time.isoformat(), tz, bt)

def hindoor(s, hl):
	decigrades = str(unhex(hl[0:4]))
	return 'ControlPanel::TempStatus %s.%s °C' % (decigrades[0:len(decigrades)-1], decigrades[len(decigrades)-1])

decoders = {
	401: htime,
	404: hindoor
}

##################################################################################################
##################################################################################################

def decode_line(s):
	canid = int(s[25:30])
	rtr = s[32]
	timestamp = datetime.datetime(int(s[0:4]), int(s[5:7]), int(s[8:10]), int(s[11:13]), int(s[14:16]), int(s[17:19]), int(s[20:23]) * 1000)

	# silently ignore RTR messages
	if rtr == 'R':
		return False
	elif rtr != ' ':
		sys.stderr.write('error: RTR flag neither "R" nor " "\n')
		return False

	if canid in decoders:
		print('V %s %s' % (timestamp.isoformat(), decoders[canid](s, s[DATA_OFFSET:])))
		return True

	print('C %s [%d] %s' % (timestamp.isoformat(), canid, s[34:]))
	return False

linenr = 1

# U, D, E, C, V
for s in sys.stdin:
	s = s.rstrip('\n')
	typechar = s[24]

	print(s)
	
	if typechar == '[':
		decode_line(s)
	elif typechar == '-':
		logd_action_char = s[48]
		if logd_action_char == 'e':
			print('D yaca-logd exiting')
		elif logd_action_char == 's':
			print('U yaca-logd starting')
		else:
			print('E error processing start/stop line %d' % linenr)
			sys.stderr.write('error processing start/stop line %d\n' % linenr)
	else:
		print('E error processing line %d' % linenr)
	linenr = linenr + 1

