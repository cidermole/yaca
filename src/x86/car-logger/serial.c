#include "serial.h"
#include "config.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct termios oldtio;

speed_t baud_const[] = { B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200 };
int baud_value[] =      { 50,  75,  110,  134,  150,  200,  300,  600,  1200,  1800,  2400,  4800,  9600,  19200,  38400,  57600,  115200 };

speed_t resolve_baud(int baud) {
	int i;
	
	for(i = 0; i < sizeof(baud_value) / sizeof(int) && baud_value[i] != baud; i++);
	if(i < sizeof(baud_value) / sizeof(int))
		return baud_const[i];
	fprintf(stderr, "could not resolve baud rate %d, defaulting to 1200\n", baud);
	return B1200;
}

int serial_init() {
	struct termios newtio;
	int fd;
	speed_t baud;

	printf("Opening device %s with %d baud\n", conf.device, conf.baud);

	if((fd = open(conf.device, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
		fprintf(stderr, "failed to open serial port %s\n", conf.device);
		exit(1);
	}

	// backup old serial port settings
	tcgetattr(fd, &oldtio);
	
	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR | IGNBRK;
	//cfmakeraw(&newtio); // ??
	
	baud = resolve_baud(conf.baud);
	if(-1 == cfsetispeed(&newtio, baud))
		fprintf(stderr, "cfsetispeed() failed: %d\n", errno);
	if(-1 == cfsetospeed(&newtio, baud))
		fprintf(stderr, "cfsetospeed() failed: %d\n", errno);
	
	if(-1 == tcflush(fd, TCIOFLUSH))
		fprintf(stderr, "tcflush() failed: %d\n", errno);
	if(-1 == tcsetattr(fd, TCSANOW, &newtio))
		fprintf(stderr, "tcsetattr() failed: %d\n", errno);

	return fd;
}

void serial_close(int fd) {
	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
}

