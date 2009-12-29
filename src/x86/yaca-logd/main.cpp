#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#include "config.h"
#include "network.h"
#include "../yaca-path.h"

int sock = 0;
FILE *yaca, *bulk;

void exit_handler(int signal) {
	struct timespec t;
	struct tm *tm;
	
	clock_gettime(CLOCK_REALTIME, &t);
	tm = localtime(&t.tv_sec);
	fprintf(yaca, "%04d-%02d-%02d %02d:%02d:%02d.%03d ------------- yaca-logd exiting -------------\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int) (t.tv_nsec / 1000000));
	fprintf(bulk, "%04d-%02d-%02d %02d:%02d:%02d.%03d ------------- yaca-logd exiting -------------\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int) (t.tv_nsec / 1000000));
	fclose(yaca);
	fclose(bulk);
#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif
	exit(0);
}

int main(int argc, char **argv) {
	int i;
	char buf[1024];
	char config_file[1024];
	Message msg;
	struct timespec t;
	struct tm *tm;
	FILE *f;
	
	init_yaca_path();
	sprintf(config_file, "%s/src/x86/yaca-logd/conf/yaca-logd.conf", yaca_path);
	load_conf(config_file);
	if((sock = connect_socket(conf.server, conf.port)) == -1)
		return 1;
	
	if(!(yaca = fopen(conf.logfile_yaca, "w"))) {
		fprintf(stderr, "Error opening yaca logfile \"%s\"\n", conf.logfile_yaca);
		goto error_close_socket;
	}

	if(!(bulk = fopen(conf.logfile_bulk, "w"))) {
		fprintf(stderr, "Error opening bulk logfile \"%s\"\n", conf.logfile_bulk);
		fclose(yaca);
		goto error_close_socket;
	}
	
	signal(SIGHUP, exit_handler);
	signal(SIGTERM, exit_handler);

	clock_gettime(CLOCK_REALTIME, &t);
	tm = localtime(&t.tv_sec);
	fprintf(yaca, "%04d-%02d-%02d %02d:%02d:%02d.%03d ------------- yaca-logd started -------------\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int) (t.tv_nsec / 1000000));
	fprintf(bulk, "%04d-%02d-%02d %02d:%02d:%02d.%03d ------------- yaca-logd started -------------\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int) (t.tv_nsec / 1000000));
	
	while(1) {
		read_message(sock, &msg);
		clock_gettime(CLOCK_REALTIME, &t);
		tm = localtime(&t.tv_sec);
		
		if((msg.id >= conf.bulk_from && msg.id <= conf.bulk_to) || (msg.id >= conf.nodeid_from && msg.id <= conf.nodeid_to))
			f = bulk;
		else
			f = yaca;
		
		fprintf(f, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%5d] %c (%d) ", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int) (t.tv_nsec / 1000000), msg.id, msg.rtr ? 'R' : ' ', msg.length);
		
		for(i = 0; i < msg.length; i++)
			fprintf(f, "%02X", msg.data[i]);
		fprintf(f, "\n");
		fflush(f);
	}
	
	fclose(yaca);
	fclose(bulk);
#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif

	return 0;
	
error_close_socket:

#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif

	return 1;
}

