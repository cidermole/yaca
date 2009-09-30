#include "socket.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#ifdef _WIN32
	/* Windows */
	#include <winsock.h>
	#include <io.h>
#else
	/* *ix */
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <unistd.h>
#endif

int socket_init() {
	struct sockaddr_in server;
	int sock;

#ifdef _WIN32
	/* init winsock */
	short wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(1, 1);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		fprintf(stderr, "failed to init winsock\n");
		exit(1);
	}
#endif

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		fprintf(stderr, "failed to create socket: %d\n", errno);
		exit(1);
	}

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(conf.port);

	if(bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		fprintf(stderr, "failed to bind socket: %d\n", errno);
		exit(1);
	}
	listen(sock, BACKLOG);

	return sock;
}

void socket_close(int fd) {
#ifdef _WIN32
	closesocket(fd);
#else
	close(fd);
#endif
}

