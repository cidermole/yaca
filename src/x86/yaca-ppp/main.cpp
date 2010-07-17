#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "config.h"
#include "network.h"
#include "../../embedded/bootloader/msgdefs.h"
#include "../../embedded/bootloader/eeprom.h"
#include "../yaca-path.h"


int main(int argc, char **argv) {
	int sock = 0, p2c[2], c2p[2], pppd_argc = 1, max, len;
	char config_file[1024], buf[sizeof(Message) * 20], *pbuf;
	char *pppd_args[100], *p, *start, ppp_name[] = "pppd"; // _params
	Message *mp, msg;
	pid_t child;
	fd_set fds;

	init_yaca_path();
	sprintf(config_file, "%s/src/x86/yaca-ppp/conf/yaca-ppp.conf", yaca_path);
	load_conf(config_file);
	msg.id = conf.tcp_out_id;
	msg.rtr = 0;

	/* load pppd args (split on ' ') */
	pppd_args[0] = ppp_name;
	for(p = conf.pppd_params, start = conf.pppd_params; p != '\0'; p++) {
		if(p == ' ') {
			pppd_args[pppd_argc++] = start;
			p = '\0';
			start = p + 1;
		}
	}
	pppd_args[pppd_argc++] = start;

	/* create pipes for communication with pppd, fork() and execvp() */
	pipe(p2c);
	pipe(c2p);
	child = fork();
	if(child == -1) {
		fprintf(stderr, "fork() failed\n");
		return 1;
	} else if(child == 0) {
		// child
		close(p2c[1]);
		close(c2p[0]);
		dup2(0, p2c[0]); // replace stdin by pipe
		dup2(1, c2p[1]); // replace stdout by pipe
		execvp("pppd", pppd_args);
		fprintf(stderr, "child: execlp() failed\n");
		return 1;
	}

	/* parent: close child's ends of pipes, connect to CAN gateway */
	close(p2c[0]);
	close(c2p[1]);
	if((sock = connect_socket(conf.server, conf.port)) == -1)
		return 1;

	while(1) {
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		FD_SET(c2p[0], &fds);
		max = sock > c2p[0] ? sock : c2p[0];
		select(max + 1, &fds, NULL, NULL, NULL);

		if(FD_ISSET(sock, &fds)) {
			len = read(sock, buf, sizeof(buf));
			pbuf = buf;
			while(len > 0) {
				if(len < sizeof(Message)) {
					printf("len < sizeof(Message)\n");
					continue;
				}
				pbuf += sizeof(Message);
				len -= sizeof(Message);
				mp = (Message *) pbuf;
				if(mp->id == config.tcp_in_id) {
					write(p2c[1], mp->data, mp->length);
				}
			}
		} else if(FD_ISSET(c2p[0], &fds)) {
			len = read(sock, buf, sizeof(buf));
			pbuf = buf;
			while(len > 0) {
				msg.info = 0;
				msg.length = (len > 8) ? 8 : len;
				memcpy(msg.data, pbuf, msg.length);
				write(sock, &msg, sizeof(Message));
				pbuf += msg.length;
				len -= msg.length;
			}
		} else {
			printf("select() returned empty... hmmm?\n");
		}
	}

#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif

	return 0;
}

