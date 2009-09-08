#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ihex.h"
#include "config.h"
#include "network.h"
#include "../../embedded/bootloader/msgdefs.h"
#include "../../embedded/bootloader/eeprom.h"
#include "../yaca-path.h"


int main(int argc, char **argv) {
	int sock = 0;
	char config_file[1024];
	bool crc_only = false;
	
	if(argc < 3) {
		printf("Too few arguments. Usage: %s <tid> <hex file> [-crc]\n", argv[0]);
		return 0;
	}
	if(argc == 4 && !strncmp(argv[3], "-crc", strlen("-crc")))
		crc_only = true;
	init_yaca_path();
	sprintf(config_file, "%s/src/x86/yaca-flash/conf/yaca-flash.conf", yaca_path);
	load_conf(config_file);
	if(!crc_only && (sock = connect_socket(conf.server, conf.port)) == -1)
		return 1;

	int tid = atoi(argv[1]);
	int d_flash_size = conf.flash_size;
	int d_page_size = conf.page_size;
	int d_bld_size = conf.boot_size;
	char str[4];

	int _d_app_pages = ((d_flash_size - d_bld_size) / d_page_size);

	char buffer[10 * 1024];
	char *current = buffer;
	int size, i, j, lastcount;
	uint16_t crc = 0xFFFF;

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
		
		write_message(sock, tid, 1, TID_BLD_ENTER);
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
