#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
//#include <string.h>

#include "ihex.h"
#include "config.h"
#include "../yaca-path.h"


int main(int argc, char **argv) {
	char config_file[1024];
	
	if(argc < 4) {
		printf("Too few arguments. Usage: %s <bootloader hex> <application hex> <output hex>\n", argv[0]);
		return 0;
	}
	init_yaca_path();
	sprintf(config_file, "%s/src/x86/yaca-flash/conf/yaca-flash.conf", yaca_path);
	load_conf(config_file);

	char bld[conf.flash_size + 1024], app[conf.flash_size + 1024];
	int out[conf.flash_size];
	int bld_size, app_size, i;

	if(!(bld_size = ihex_parse(bld, sizeof(bld), argv[1])))
		return 1;
	if(bld_size > conf.flash_size)
		fprintf(stderr, "WARNING: hex file %s (%d bytes) is larger than MCU flash (%d bytes)\n", argv[1], bld_size, conf.flash_size);

	if(!(app_size = ihex_parse(app, sizeof(app), argv[2])))
		return 1;
	if(app_size > conf.flash_size - conf.boot_size)
		fprintf(stderr, "WARNING: hex file %s (%d bytes) is larger than MCU flash (%d bytes)\n", argv[2], app_size, conf.flash_size - conf.boot_size);

	for(i = 0; i < conf.flash_size - conf.boot_size; i++)
		out[i] = app[i];
	for(i = conf.flash_size - conf.boot_size; i < conf.flash_size; i++)
		out[i] = bld[i];
	
	if(!ihex_write(out, conf.flash_size, argv[3]))
		return 1;
	
	return 0;
}

