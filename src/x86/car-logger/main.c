#include "config.h"
#include "serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int state = 0, quit = 0, uart;

void sigterm_handler(int signal) {
	printf("signal %ld received, exiting.\n", signal);
	serial_close(uart);
	if(state == 0)
		exit(0);
	else
		quit = 1;
}

void read_line(int fd, char *buf, int size) {
	char c;
	int bytes, i;

	bytes = read(fd, &c, 1);
	for(i = 0; bytes != 0 && c != '\n' && i < size; i++) {
		*buf++ = c;
		bytes = read(fd, &c, 1);
	}
	if(i > 0)
		*(--buf) = '\0'; // overwrite '\r' with end of string
	else
		*buf = '\0'; // empty string
}

void write_str(int fd, char *s) {
	int l = strlen(s);
	write(fd, s, l);
	tcdrain(uart);
}

int main(int argc, char **argv) {
	int i;
	pid_t pid;
	char line[1024];

	if(argc < 2) {
		printf("Usage: %s <config file>\n", argv[0]);
		return 0;
	}
	load_conf(argv[1]);
	uart = serial_init();
	
	if(conf.debug == 0) {
		pid = fork();
		if(pid < 0) {
			fprintf(stderr, "fork() failed\n");
			return 1;
		} else if(pid > 0) { // parent
			return 0;
		}
	}
	
	signal(SIGTERM, sigterm_handler);
	signal(SIGINT, sigterm_handler);
	signal(SIGPIPE, SIG_IGN); // instead of SIGPIPE, get socket write errors as -1 / EPIPE


/*

example count output:

"
!Event detection stop

!Counted detections: 112

!Event detection run
"

*/

	while(!quit) {
		usleep(1000*1000*2);

		state = 1;
		write_str(uart, "\x1B" "C"); // ESC, C (get count)

		for(i = 0; i < 3; i++) {
			read_line(uart, line, sizeof(line));
			if(conf.debug >= 1)
				printf("%s\n", line);
		}

		read_line(uart, line, sizeof(line));
		printf("interesting line: %s\n", line);

		for(i = 0; i < 3; i++) {
			read_line(uart, line, sizeof(line));
			if(conf.debug >= 1)
				printf("%s\n", line);
		}
		state = 0;
	}

	serial_close(uart);

	return 0;
}

