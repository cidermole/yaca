#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include "ihex.h"
#include "config.h"
#include "network.h"
#include "../../embedded/bootloader/msgdefs.h"
#include "../../embedded/bootloader/eeprom.h"
#include "../yaca-path.h"

#define ATMEGA8_SIGNATURE "1E9307"

// returns bootloader version
int get_mcu_signature(char *signature, int sock, int tid) {
	fd_set fds;
	struct timeval tv;
	struct timespec t, till;
	struct Message msg;
	
	// try to get MCU signature (works with bootloader version 2 or above)
	write_message(sock, tid, 1, TID_BLD_GETSIG);
	
	// wait for response
	clock_gettime(CLOCK_REALTIME, &till);
	clock_gettime(CLOCK_REALTIME, &t);
	till.tv_sec += 2; // 1-2 sec. timeout
	
	while(t.tv_sec < till.tv_sec) {
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		tv.tv_sec = 1; // select() timeout
		tv.tv_usec = 0;
		select(sock + 1, &fds, NULL, NULL, &tv);
		if(FD_ISSET(sock, &fds)) {
			read_message(sock, &msg);
			if(msg.id == tid && msg.data[0] == TID_BLD_SIG) {
				sprintf(signature, "%02X%02X%02X", msg.data[1], msg.data[2], msg.data[3]);
				return msg.data[4];
			} else if(msg.id == tid) {
				fprintf(stderr, "unexpected message type %d received while getting MCU signature\n", (int) msg.data[0]);
			}
		}
		clock_gettime(CLOCK_REALTIME, &t);
	}
	
	// timeout
	return 0;
}

int main(int argc, char **argv) {
	int sock = 0;
	char common_config_file[1024];
	char config_file[1024];
	bool crc_only = false, app = false;
	
	if(argc < 3) {
		printf("Too few arguments. Usage: %s <tid> <hex file> [-crc|-app]\n", argv[0]);
		return 0;
	}
	if(argc == 4 && !strncmp(argv[3], "-crc", strlen("-crc")))
		crc_only = true;
	if(argc == 4 && !strncmp(argv[3], "-app", strlen("-app")))
		app = true;
	init_yaca_path();
	sprintf(common_config_file, "%s/src/x86/yaca-flash/conf/yaca-flash.conf", yaca_path);
	load_common_conf(common_config_file);
	if(!crc_only && (sock = connect_socket(common_conf.server, common_conf.port)) == -1)
		return 1;

	int tid = atoi(argv[1]);
	int d_flash_size;
	int d_page_size;
	int d_bld_size;
	char str[4];

	int _d_app_pages;

	char buffer[10 * 1024];
	char signature[1024];
	int version = 0;
	char *current = buffer;
	int size, i, j, lastcount;
	uint16_t crc = 0xFFFF;

	if(!crc_only) {
		printf("Starting bootloader and auto-detecting MCU type...\n");

		write_message(sock, tid, 1, TID_BLD_ENTER);
		if(app) {
			printf("-app given, waiting for application to enter bootloader...\n");
			usleep(1500 * 1000);
		}

		if(!(version = get_mcu_signature(signature, sock, tid))) {
			fprintf(stderr, "WARNING: MCU signature detection failed. Assuming the bootloader version is too old.\n");
			fprintf(stderr, "Enter MCU signature (" ATMEGA8_SIGNATURE " for ATmega8): \n");
			fflush(stdin);
			scanf("%6s", signature);
		}
	}
	sprintf(config_file, "%s/src/x86/yaca-flash/conf/avr_%s.conf", yaca_path, signature);
	load_conf(config_file);
	if(!crc_only)
		printf("Loaded config file for MCU signature %s (%s bootloader version %d)\n", signature, conf.mcu, version);
	d_flash_size = conf.flash_size;
	d_page_size = conf.page_size;
	d_bld_size = conf.boot_size;
	_d_app_pages = ((d_flash_size - d_bld_size) / d_page_size);

	if(size = ihex_parse(buffer, sizeof(buffer), argv[2])) {
		if(size > _d_app_pages * d_page_size && !crc_only) {
			fprintf(stderr, "WARNING: hex file (%d bytes) is larger than MCU flash (%d bytes)\n", size, (_d_app_pages * d_page_size));
			fprintf(stderr, "Enter 'yes' to force flashing, truncating off the end of the hex file: \n");
			fflush(stdin);
			scanf("%3s", str);
			if(strncmp(str, "yes", 3))
				return 1;
		}
		for(i = 0; i < _d_app_pages * d_page_size; i++)
			crc = crc16_update(crc, buffer[i]);
		if(crc_only) {
			printf("0x%04X", (unsigned int) crc);
			return 0;
		}
		
		write_message(sock, tid, 3, TID_BLD_PAGESEL, 0, 0);

		for(i = 0; i < 49; i++)
			printf(" ");
		printf("v\n");

		for(i = 0, lastcount = 0; i < _d_app_pages; i++) {
			current = target_flash_page(current, d_page_size, tid, sock);
			if((i * 50) / _d_app_pages > lastcount) {
				for(j = lastcount; j < (i * 50) / _d_app_pages; j++) {
					printf("#");
					fflush(stdout);
				}
				lastcount = (i * 50) / _d_app_pages;
			}
		}
		printf("\n");
		
		// TODO: check for reply if bootloader version transmits ACKs
		printf("Rewriting target EEPROM (clearing error flags, setting CRC = 0x%04X)...\n", crc);
		target_eeprom_write(tid, sock, ((int) EE_CRC16), crc & 0xFF);
		target_eeprom_write(tid, sock, ((int) EE_CRC16) + 1, crc >> 8);
		target_eeprom_write(tid, sock, ((int) EE_ERR), 0);
		
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

