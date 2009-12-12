#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "network.h"
#include "../yaca-path.h"

int main(int argc, char **argv) {
	int sock = 0;
	char config_file[1024];
	Message msg;
	
	init_yaca_path();
	sprintf(config_file, "%s/src/x86/mpmon/conf/mpmon.conf", yaca_path);
	load_conf(config_file);
	if((sock = connect_socket(conf.server, conf.port)) == -1)
		return 1;
	
	while(1) {
		if(!read_message(sock, &msg)) {
			printf("read_message() returned an error, sleeping 1 s...\n");
			usleep(1000 * 1000);
			continue;
		}
		if(msg.id != conf.status_canid || msg.rtr)
			continue;
		
		printf("AC status: %d\n", msg.data[0]);
	}

#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif

	return 0;
}

