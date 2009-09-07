#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
	/* Windows */
	#include <winsock.h>
	#include <io.h>
#else
	/* *ix */
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <unistd.h>
#endif
#include "ihex.h"

#include "../../embedded/bootloader/msgdefs.h"
#include "../../embedded/bootloader/eeprom.h"

struct Message {
        uint8_t info;
        uint32_t id;
        uint8_t rtr;
        uint8_t length;
        uint8_t data[8];
} __attribute__((__packed__));

int read_message(int sock, struct Message *buffer) {
	int sum = 0;
	ssize_t rv = 0;

	while(sum < sizeof(struct Message) && rv != -1) {
		if((rv = read(sock, buffer, sizeof(struct Message))) != -1)
			sum += rv;
	}

	return (rv != -1);
}

char *flash_page(const char *buffer, int page_size, int tid, int sock) {
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

void _target_eeprom_write(uint32_t id, int sock, uint16_t addr, uint8_t data) {
	struct Message msg;
	
	memset(&msg, 0, sizeof(msg));
	msg.data[0] = TID_BLD_EE_WR;
	msg.data[1] = addr & 0xFF;
	msg.data[2] = addr >> 8;
	msg.data[3] = data;
	msg.length = 4;
	msg.id = id;
	
	write(sock, &msg, sizeof(msg));
}

int main(int argc, char **argv) {
	struct hostent *host;
	struct sockaddr_in addr;
	int sock = 0;
	
	if(argc < 2) {
		printf("Too few arguments. Usage: %s <hex file>\n", argv[0]);
		return 0;
	}

	// TODO: conf
	if(!inet_aton("192.168.1.1", &addr.sin_addr)) {
		if(!(host = gethostbyname("192.168.1.1"))) {
			fprintf(stderr, "gethostbyname() failed\n");
			return 1;
		}
		addr.sin_addr = *(struct in_addr*)host->h_addr;
	}

	if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "failed to create socket\n");
		return 1;
	}
	addr.sin_port = htons(1222); // TODO: conf
	addr.sin_family = AF_INET;

	if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "failed to connect()\n");
		return 1;
	}

	// TODO: set these according to MCU type
	int tid = 0x01;
	int d_flash_size = 8 * 1024;
	int d_page_size = 64;
	int d_bld_size = 2 * 1024;
	char str[4];

	int _d_app_pages = ((d_flash_size - d_bld_size) / d_page_size);

	char buffer[10 * 1024];
	char *current = buffer;
	int size, i, j, lastcount;
	uint16_t crc = 0xFFFF;

	if(size = ihex_parse(buffer, sizeof(buffer), argv[1])) {
		if(size > _d_app_pages * d_page_size) {
			fprintf(stderr, "WARNING: hex file (%d bytes) is larger than MCU flash (%d bytes)\n", size, (_d_app_pages * d_page_size));
			fprintf(stderr, "Enter 'yes' to force flashing, truncating off the end of the hex file: \n");
			fflush(stdin);
			scanf("%3s", str);
			if(strncmp(str, "yes", 3))
				return 1;
		}
		for(i = 0; i < _d_app_pages * d_page_size; i++)
			crc = crc16_update(crc, buffer[i]);
		
		struct Message enter;
		memset(&enter, 0, sizeof(enter));
		enter.id = tid;
		enter.data[0] = TID_BLD_ENTER;
		enter.length = 1;
		write(sock, &enter, sizeof(enter));

		enter.data[0] = TID_BLD_PAGESEL;
		enter.data[1] = 0;
		enter.data[2] = 0;
		enter.length = 3;
		write(sock, &enter, sizeof(enter));

		for(i = 0; i < 49; i++)
			printf(" ");
		printf("v\n");

		for(i = 0, lastcount = 0; i < _d_app_pages; i++) {
			current = flash_page(current, d_page_size, tid, sock);
			if((i * 50) / _d_app_pages > lastcount) {
				for(j = lastcount; j < (i * 50) / _d_app_pages; j++) {
					printf("#");
					fflush(stdout);
				}
				lastcount = (i * 50) / _d_app_pages;
			}
		}
		printf("\n");
		
		printf("Rewriting target EEPROM (clearing error flags, setting CRC = 0x%04X)...\n", crc);
		_target_eeprom_write(tid, sock, ((uint16_t) EE_CRC16), crc & 0xFF);
		_target_eeprom_write(tid, sock, ((uint16_t) EE_CRC16) + 1, crc >> 8);
		_target_eeprom_write(tid, sock, ((uint16_t) EE_ERR), 0);
		
		printf("Done. Terminating in 1 s...\n"); // TODO: some better flush?
		sleep(1);
	}

#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif

	return 0;
}
