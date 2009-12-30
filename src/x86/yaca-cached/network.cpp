#ifdef _WIN32
	#include <winsock.h>
	#include <io.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "../../embedded/bootloader/msgdefs.h"


int connect_socket(const char* host, int port) {
	struct hostent *hoste;
	struct sockaddr_in addr;
	int sock = -1;
	
	if(!inet_aton(host, &addr.sin_addr)) {
		if(!(hoste = gethostbyname(host))) {
			fprintf(stderr, "gethostbyname() failed\n");
			return -1;
		}
		addr.sin_addr = *(struct in_addr *) hoste->h_addr;
	}

	if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket() failed\n");
		return -1;
	}
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	if(connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		fprintf(stderr, "connect(host %s, port %d) failed\n", host, port);
		return -1;
	}
	return sock;
}

////////////////////////////////////////////////////////////////////////////////

void write_message(int sock, unsigned int id, int length, char d0, char d1, char d2, char d3, char d4, char d5, char d6, char d7) {
		struct Message msg;
		
		memset(&msg, 0, sizeof(msg));
		msg.id = id;
		msg.length = length;
		msg.data[0] = d0;
		msg.data[1] = d1;
		msg.data[2] = d2;
		msg.data[3] = d3;
		msg.data[4] = d4;
		msg.data[5] = d5;
		msg.data[6] = d6;
		msg.data[7] = d7;
		write(sock, &msg, sizeof(msg));
}

////////////////////////////////////////////////////////////////////////////////

int read_message(int sock, struct Message *buffer) {
	int sum = 0;
	ssize_t rv = 0;
	char *p = (char *) buffer;

	while(sum < sizeof(struct Message) && rv != -1) {
		if((rv = read(sock, p, sizeof(struct Message) - sum)) != -1) {
			sum += rv;
			p += rv;
		}
	}

	return (rv != -1);
}

////////////////////////////////////////////////////////////////////////////////

int poll_message(int sock) {
	fd_set fds;
	struct timeval timeout = {RTR_TIMEOUT_SEC, RTR_TIMEOUT_US};
	
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	select(sock + 1, &fds, NULL, NULL, &timeout);
	return FD_ISSET(sock, &fds);
}

////////////////////////////////////////////////////////////////////////////////

char *target_flash_page(const char *buffer, int page_size, int tid, int sock) {
	struct Message enter, in;
	int i, singleBytes;
	static int page = 0;

	memset(&enter, 0, sizeof(enter));
	enter.id = tid;

	enter.data[0] = TID_BLD_DATA;
	for(i = 0, singleBytes = 1; i < page_size; i++) {
		if(singleBytes >= 8) {
			enter.length = 8;
			write(sock, &enter, sizeof(enter));
			singleBytes = 1;
		}
		enter.data[singleBytes++] = *(buffer++);
	}
	enter.length = singleBytes;
	write(sock, &enter, sizeof(enter));

	in.id = 0;
	while(in.id != tid) {
		if(!read_message(sock, &in)) {
			fprintf(stderr, "\nERROR reading from socket in read_message()\n");
			exit(1);
		}
	}
	if(in.data[0] != 0x0D) {
		fprintf(stderr, "\nERROR: Protocol error while receiving from programmed node\n");
		exit(1);
	}
	return (char *)buffer;
}

////////////////////////////////////////////////////////////////////////////////

uint16_t crc16_update(uint16_t crc, uint8_t c) {
	int i;
	
	crc ^= c;
	for(i = 0; i < 8; i++) {
		if(crc & 1)
			crc = (crc >> 1) ^ 0xA001;
		else
			crc = (crc >> 1);
	}
	
	return crc;
}

////////////////////////////////////////////////////////////////////////////////

void target_eeprom_write(uint32_t id, int sock, uint16_t addr, uint8_t data) {
	struct Message msg;
	
	memset(&msg, 0, sizeof(msg));
	msg.data[0] = TID_BLD_EE_WR;
	msg.data[1] = addr & 0xFF;
	msg.data[2] = addr >> 8;
	msg.data[3] = data;
	msg.length = 4;
	msg.id = id;
	
	write(sock, &msg, sizeof(msg));
	usleep(10 * 1000);
}

////////////////////////////////////////////////////////////////////////////////

int create_host(int port) {
    int s, c;
    socklen_t addr_len;
    struct sockaddr_in addr;

    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        perror("socket() failed");
        return -1;
    }

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("bind() failed");
        return -2;
    }

    if (listen(s, 3) == -1)
    {
        perror("listen() failed");
        return -3;
    }

    return s;
}

