#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <pty.h>
#include <ctype.h>

#include "config.h"
#include "network.h"
#include "../../embedded/bootloader/msgdefs.h"
#include "../../embedded/bootloader/eeprom.h"
#include "../yaca-path.h"

void printmsg(struct Message *m) {
	int i;
	printf(" ");
	for(i = 0; i < m->length; i++) {
		printf("%02X", m->data[i]);
	}
	printf(" ");
	for(i = 0; i < m->length; i++) {
		printf("%c", m->data[i] < 0x80 && isprint(m->data[i]) ? m->data[i] : '?');
	}
	printf("\n");
	fflush(stdout);
}

int main(int argc, char **argv) {
	int sock = 0, tty, pppd_argc = 1, max, len;
	char config_file[1024], buf[sizeof(struct Message) * 20], *pbuf;
	char *pppd_args[100], *p, *start, ppp_name[] = "/usr/sbin/pppd"; // _params
	struct Message *mp, msg;
	struct timeval timeout;
	pid_t child, temp_pid, pid;
	fd_set fds;

	init_yaca_path();
	sprintf(config_file, "%s/src/x86/yaca-ppp/conf/yaca-ppp.conf", yaca_path);
	load_conf(config_file);
	msg.id = conf.tcp_out_id;
	msg.rtr = 0;

	/* load pppd args (split on ' ') */
	pppd_args[0] = ppp_name;
	for(p = conf.pppd_params, start = conf.pppd_params; *p != '\0'; p++) {
		if(*p == ' ') {
			pppd_args[pppd_argc++] = start;
			*p = '\0';
			start = p + 1;
		}
	}
	pppd_args[pppd_argc++] = start;
	pppd_args[pppd_argc] = NULL;

	/* fork daemon */
	pid = fork();
	if(pid < 0) {
		fprintf(stderr, "fork() failed\n");
		return 1;
	} else if(pid > 0) { // parent
		return 0;
	}

	/* create pseudo-tty for communication with pppd, fork() and execvp() */
	child = forkpty(&tty, NULL, NULL, NULL);
	if(child == -1) {
		fprintf(stderr, "fork() failed\n");
		return 1;
	} else if(child == 0) {
		// child
		execv(ppp_name, pppd_args);
		fprintf(stderr, "child: execv() failed: %d\n", errno);
		return 1;
	}

	/* parent: connect to CAN gateway */
	if((sock = connect_socket(conf.server, conf.port)) == -1)
		return 1;

	while(1) {
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		FD_SET(tty, &fds);
		max = sock > tty ? sock : tty;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		select(max + 1, &fds, NULL, NULL, &timeout); // timeout: wake up every second

		if(FD_ISSET(sock, &fds)) {
			len = read(sock, buf, sizeof(buf));
			pbuf = buf;
			while(len > 0) {
				if(len < sizeof(struct Message)) {
					printf("len < sizeof(struct Message)\n"); /* FIXME */
					continue;
				}
				mp = (struct Message *) pbuf;
				if(mp->id == conf.tcp_in_id) {
					write(tty, mp->data, mp->length);
					//printf(">"); printmsg(mp);
				}
				pbuf += sizeof(struct Message);
				len -= sizeof(struct Message);
			}
		} else if(FD_ISSET(tty, &fds)) {
			len = read(tty, buf, sizeof(buf));
			pbuf = buf;
			while(len > 0) {
				msg.info = 0;
				msg.length = (len > 8) ? 8 : len;
				memcpy(msg.data, pbuf, msg.length);
				write(sock, &msg, sizeof(struct Message));
				//printf("<"); printmsg(&msg);
				pbuf += msg.length;
				len -= msg.length;
			}
		}

		temp_pid = waitpid(child, NULL, WNOHANG); // check if pppd is still running

		if(temp_pid > 0) { // pppd exited
			close(tty);

			usleep(1000000 * 10); // wait 10 s - prevent 'endless-pppd' loop with 100% CPU (seems to have occured?)

			/* create pseudo-tty for communication with pppd, fork() and execvp() */
			child = forkpty(&tty, NULL, NULL, NULL);
			if(child == -1) {
				fprintf(stderr, "fork() failed\n");
				return 1;
			} else if(child == 0) {
				// child
				execv(ppp_name, pppd_args);
				fprintf(stderr, "child: execv() failed: %d\n", errno);
				return 1;
			}
		} else if(temp_pid == -1) { // error
			fprintf(stderr, "waitpid() returned -1, errno %d\n", errno);
		}
	}

#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif

	return 0;
}

