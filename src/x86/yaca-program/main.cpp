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


int get_real_eeprom_bytes(int *eeprom, int size) {
	int i, b = 0;
	
	for(i = 0; i < size; i++)
		if(eeprom[i] != -1)
			b++;
	
	return b;
}

bool set_config(int *eeprom, int size, string type, int addr, XmlTree *setting, const char *xmlfile) {
	long long i;
	char *p;
	
	assert(addr >= 0);
	
	if(type == "int32") {
		i = strtoul(setting->attribute("value").c_str(), &p, 0);
		if(*p != '\0') {
			fprintf(stderr, "Error parsing XML file %s: setting value \"%s\" can't be converted\n", xmlfile, setting->attribute("value").c_str());
			return false;
		}
		assert(addr + 4 <= size);
		assert(eeprom[addr] == -1); eeprom[addr++] = (i >> (8 * 0)) & 0xff;
		assert(eeprom[addr] == -1); eeprom[addr++] = (i >> (8 * 1)) & 0xff;
		assert(eeprom[addr] == -1); eeprom[addr++] = (i >> (8 * 2)) & 0xff;
		assert(eeprom[addr] == -1); eeprom[addr++] = (i >> (8 * 3)) & 0xff;
		
	} else if(type == "int16") {
		i = strtoul(setting->attribute("value").c_str(), &p, 0);
		if(*p != '\0') {
			fprintf(stderr, "Error parsing XML file %s: setting value \"%s\" can't be converted\n", xmlfile, setting->attribute("value").c_str());
			return false;
		}
		assert(addr + 2 <= size);
		assert(eeprom[addr] == -1); eeprom[addr++] = (i >> (8 * 0)) & 0xff;
		assert(eeprom[addr] == -1); eeprom[addr++] = (i >> (8 * 1)) & 0xff;

	} else if(type == "int8") {
		i = strtoul(setting->attribute("value").c_str(), &p, 0);
		if(*p != '\0') {
			fprintf(stderr, "Error parsing XML file %s: setting value \"%s\" can't be converted\n", xmlfile, setting->attribute("value").c_str());
			return false;
		}
		assert(addr + 1 <= size);
		assert(eeprom[addr] == -1); eeprom[addr++] = (i >> (8 * 0)) & 0xff;

	} else {
		fprintf(stderr, "Error parsing XML file %s: no such setting type \"%s\"\n", xmlfile, type.c_str());
		return false;
	}
	return true;
}

#define ROW_WIDTH 16
void debug_print_eeprom(int *eeprom, int size) {
	int i, j;
	
	printf("Dumping %d bytes of EEPROM:\n", size);
	for(i = 0; i < size / ROW_WIDTH; i++) {
		printf("%03X:", i * ROW_WIDTH);
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
		printf("Too few arguments. Usage: %s <nds xml> <config xml> [-new <crc> <output hex file>]\n", argv[0]);
		return 0;
	} else if(argc > 3 && argc < 6) {
		printf("Too few arguments. Usage with more params: %s <nds xml> <config xml> -new <crc> <output hex file>\n", argv[0]);
		return 0;
	}
	nds_fname = argv[1];
	config_fname = argv[2];
	
	init_yaca_path();
	sprintf(config_file, "%s/src/x86/yaca-program/conf/yaca-program.conf", yaca_path);
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

	assert((*nds.begin())->name() == "nds");
	assert((*config.begin())->name() == "node-config");
	tid = strtol((*config.begin())->attribute("id").c_str(), &p, 0);
	if(*p != '\0') {
		fprintf(stderr, "Error parsing XML file %s: can't convert id attribute of node-config: not numeric\n", config_fname);
		return 1;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////

	int eeprom[conf.eeprom_size], i_eep = conf.canid_table_begin, addr;
	list<XmlTree *>::iterator msg_nds, msg_conf, messages_in, messages, xeeprom, setting, settings, child;
	string type;
	
	memset(eeprom, 0xFF, sizeof(eeprom));
	
	for(messages_in = (*nds.begin())->begin(); messages_in != (*nds.begin())->end() && (*messages_in)->name() != "messages-in"; messages_in++);
	assert((*messages_in)->name() == "messages-in");
	for(messages = (*config.begin())->begin(); messages != (*config.begin())->end() && (*messages)->name() != "messages"; messages++);
	assert((*messages)->name() == "messages");

	// Build CANid table by resolving messages in the order given in the nds, looking them up in the config
	// See src/embedded/libyaca/dispatch.c for details on eeprom structure: [2][CANid][CANid][1][CANid][3][CANid][CANid][CANid]
	//
	for(msg_nds = (*messages_in)->begin(); msg_nds != (*messages_in)->end(); msg_nds++) {
		assert((*msg_nds)->name() == "msg");
		// Find a message id definition in the config
		for(msg_conf = (*messages)->begin(); msg_conf != (*messages)->end() && (*msg_conf)->attribute("name") != (*msg_nds)->attribute("name"); msg_conf++);
		
		if(msg_conf != (*messages)->end() && (*msg_conf)->attribute("name") == (*msg_nds)->attribute("name")) {
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
	
	// Reset error status variable in eeprom
	eeprom[(unsigned int) EE_ERR] = 0;
	
	if(argc == 6) {
		i = strtol(argv[4], &p, 0);
		if(*p != '\0') {
			fprintf(stderr, "Error converting CRC \"%s\" to a number\n", argv[4]);
			return 1;
		}
		eeprom[((unsigned int) EE_CRC16) + 0] = (i >> (8 * 0)) & 0xff;
		eeprom[((unsigned int) EE_CRC16) + 1] = (i >> (8 * 1)) & 0xff;

		eeprom[((unsigned int) EE_TEMPID) + 0] = (tid >> (8 * 0)) & 0xff;
		eeprom[((unsigned int) EE_TEMPID) + 1] = (tid >> (8 * 1)) & 0xff;
		eeprom[((unsigned int) EE_TEMPID) + 2] = (tid >> (8 * 2)) & 0xff;
		eeprom[((unsigned int) EE_TEMPID) + 3] = (tid >> (8 * 3)) & 0xff;
	}
	
	// Single config variables
	for(xeeprom = (*nds.begin())->begin(); xeeprom != (*nds.begin())->end() && (*xeeprom)->name() != "eeprom"; xeeprom++);
	assert((*xeeprom)->name() == "eeprom");
	for(settings = (*config.begin())->begin(); settings != (*config.begin())->end() && (*settings)->name() != "settings"; settings++);
	assert((*settings)->name() == "settings");
	
	for(setting = (*xeeprom)->begin(); setting != (*xeeprom)->end(); setting++) {
		assert((*setting)->name() == "setting");
		type = (*setting)->attribute("type");
		addr = strtol((*setting)->attribute("address").c_str(), &p, 0);
		if(*p != '\0') {
			fprintf(stderr, "Error parsing XML file %s: can't convert address of <setting> tag: not numeric\n", nds_fname);
			return 1;
		}
		// Match config
		for(child = (*settings)->begin(); child != (*settings)->end() && (*child)->attribute("name") != (*setting)->attribute("name"); child++);
		if(child != (*settings)->end() && (*child)->attribute("name") == (*setting)->attribute("name")) {
			if(!set_config(eeprom, conf.eeprom_size, type, addr, (*child), config_fname))
				return 1;
		}
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
	//debug_print_eeprom(eeprom, conf.eeprom_size);
	
	if(argc == 6) {
		ihex_write(eeprom, conf.eeprom_size, argv[5]);
	} else {
		// TODO: program
	}

	if(argc != 6) {	
		printf("Done. Terminating in 1 s...\n"); // TODO: some better flush?
		sleep(1);
	}

	if(sock != -1)
#ifdef _WIN32
		closesocket(sock);
#else
		close(sock);
#endif

	return 0;
}
