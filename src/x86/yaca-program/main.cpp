#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "ihex.h"
#include "config.h"
#include "network.h"
#include "../../embedded/bootloader/msgdefs.h"
#include "../../embedded/bootloader/eeprom.h"
#include "../yaca-path.h"
#include "XmlTree.h"

using namespace std;


#define ROW_WIDTH 16
void debug_print_eeprom(int *eeprom, int size) {
	int i, j;
	
	printf("Dumping %d bytes of EEPROM:\n", size);
	for(i = 0; i < size / ROW_WIDTH; i++) {
		printf("%03X:", i);
		for(j = 0; j < ROW_WIDTH; j++)
			if(eeprom[i * ROW_WIDTH + j] == -1)
				printf(" xx");
			else
				printf(" %02X", eeprom[i * ROW_WIDTH + j]);
		printf("\n");
	}
}

int main(int argc, char **argv) {
	int sock = -1;
	char config_file[1024], *p, *nds_fname, *config_fname;
	XmlTree nds, config;
	int tid = -1, i;
	
	if(argc < 3) {
		printf("Too few arguments. Usage: %s <nds xml> <config xml> [-new]\n", argv[0]);
		return 0;
	}
	nds_fname = argv[1];
	config_fname = argv[2];
	
	init_yaca_path();
	sprintf(config_file, "%s/src/x86/yaca-flash/conf/yaca-program.conf", yaca_path);
	load_conf(config_file);

	try {
		nds.read(nds_fname);
	} catch(XmlError err) {
		fprintf(stderr, "Error parsing XML file %s: XmlTree error %d (%s)\n", nds_fname, err.code, err.desc);
		return 1;
	}
	try {
		config.read(config_fname);
	} catch(XmlError err) {
		fprintf(stderr, "Error parsing XML file %s: XmlTree error %d (%s)\n", config_fname, err.code, err.desc);
		return 1;
	}

	assert(nds.name() == "nds");
	assert(config.name() == "node-config");
	tid = strtol(config.attribute("id").c_str(), &p, 0);
	if(*p != '\0') {
		fprintf(stderr, "Error parsing XML file %s: can't convert id attribute of node-config: not numeric\n", config_fname);
		return 1;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////

	int eeprom[conf.eeprom_size], i_eep = conf.canid_table_begin;
	list<XmlTree *>::iterator msg_nds, msg_conf, messages_in, messages, child;
	
	memset(eeprom, 0xFF, sizeof(eeprom));
	
	for(messages_in = nds.begin(); messages_in != nds.end() && (*messages_in)->name() != "messages-in"; messages_in++);
	assert((*messages_in)->name() == "messages-in");
	for(messages = config.begin(); messages != config.end() && (*messages)->name() != "messages"; messages++);
	assert((*messages)->name() == "messages");

	// Build CANid table by resolving messages in the order given in the nds, looking them up in the config
	// See src/embedded/libyaca/dispatch.c for details on eeprom structure: [2][CANid][CANid][1][CANid][3][CANid][CANid][CANid]
	//
	for(msg_nds = (*messages_in)->begin(); msg_nds != (*messages_in)->end(); msg_nds++) {
		assert((*msg_nds)->name() == "msg");
		// Find a message id definition in the config
		for(msg_conf = (*messages)->begin(); msg_conf != (*messages)->end() && (*msg_conf)->attribute("name") != (*msg_nds)->attribute("name"); msg_conf++);
		
		if((*msg_conf)->attribute("name") == (*msg_nds)->attribute("name")) {
			// Writing the number of CANids -> [2][CANid][CANid]
			assert(i_eep < conf.eeprom_size);
			eeprom[i_eep++] = (*msg_conf)->size();
			
			// Write all CANids of a message definition from the config to EEPROM
			for(child = (*msg_conf)->begin(); child != (*msg_conf)->end(); child++) {
				assert((*child)->name() == "id");
				i = strtol((*child)->text().c_str(), &p, 0);
				if(*p != '\0') {
					fprintf(stderr, "Error parsing XML file %s: can't convert text of <id> tag: not numeric\n", config_fname);
					return 1;
				}
				assert(i_eep + 4 <= conf.eeprom_size);
				eeprom[i_eep++] = (i >> (8 * 0)) & 0xff;
				eeprom[i_eep++] = (i >> (8 * 1)) & 0xff;
				eeprom[i_eep++] = (i >> (8 * 2)) & 0xff;
				eeprom[i_eep++] = (i >> (8 * 3)) & 0xff;
			}
		} else {
			// Writing the number of CANids -> [0] [2][CANid][CANid] ...
			// no message definitions found in config
			assert(i_eep < conf.eeprom_size);
			eeprom[i_eep++] = 0;
		}
	}
	
	debug_print_eeprom(eeprom, conf.eeprom_size);
	
	// TODO: single config values

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	// only if not writing to ihex
	if((sock = connect_socket(conf.server, conf.port)) == -1)
		return 1;
	
	if(argc == 4 && !strncmp(argv[3], "-new", strlen("-new"))
		tid = 0x1FFFFFFF;
	// TODO: don't sed tid to ffff, which is bullshit (extra work, you need to connect the programmer and single can), need to create intel hex for avrdude
	*/

/*
	if(size = ihex_parse(buffer, sizeof(buffer), argv[2])) {
		if(size > _d_eeprom_size) {
			fprintf(stderr, "WARNING: hex file (%d bytes) is larger than MCU eeprom (%d bytes)\n", size, _eeprom_size);
			fprintf(stderr, "Enter 'yes' to force writing, truncating off the end of the hex file: \n");
			fflush(stdin);
			scanf("%3s", str);
			if(strncmp(str, "yes", 3))
				return 1;
		}
		
		// TODO
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
		

	}*/
	
	printf("Done. Terminating in 1 s...\n"); // TODO: some better flush?
	sleep(1);

	if(sock != -1)
#ifdef _WIN32
		closesocket(sock);
#else
		close(sock);
#endif

	return 0;
}
