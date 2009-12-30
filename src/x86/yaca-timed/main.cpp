#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>

//#include "config.h"
#include "network.h"
#include "../yaca-path.h"

int main(int argc, char **argv) {
	int sock = 0, flags, pid;
	char buf[1024];
//	char config_file[1024];
	Message msg;
	struct timespec t, d;
	struct tm *tm;
	
	init_yaca_path();
//	sprintf(config_file, "%s/src/x86/mpmon/conf/mpmon.conf", yaca_path);
//	load_conf(config_file);
//	if((sock = connect_socket(conf.server, conf.port)) == -1)
	if((sock = connect_socket("192.168.1.1", 1222)) == -1)
		return 1;

	if((flags = fcntl(sock, F_GETFD)) == -1) {
		fprintf(stderr, "fcntl(sock, F_GETFD) failed\n");
		return 1;
	}
	// set O_NONBLOCK on 'sock'
	if(fcntl(sock, F_SETFD, flags | O_NONBLOCK) == -1) {
		fprintf(stderr, "fcntl(sock, F_SETFD, flags | O_NONBLOCK) failed\n");
		return 1;
	}
	
	pid = fork();
	if(pid < 0) {
		fprintf(stderr, "fork() failed\n");
		return 1;
	} else if(pid > 0) { // parent
		return 0;
	}
	
	d.tv_sec = 0;
	
	while(1) {
		clock_gettime(CLOCK_REALTIME, &t);
		d.tv_nsec = 1000000000 - t.tv_nsec;
		nanosleep(&d, NULL);
		
		t.tv_sec++;
		tm = localtime(&t.tv_sec);
		write_message(sock, /* TODO */ 401, 8, tm->tm_hour, tm->tm_min, tm->tm_sec, (tm->tm_year + 1900) >> 8, (uint8_t) (tm->tm_year + 1900), tm->tm_mon + 1, tm->tm_mday, tm->tm_isdst);
		read(sock, buf, sizeof(buf));
	}

#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif

	return 0;
}

