#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"


int connect_socket(const char* host, int port) {
	struct hostent *hoste;
	struct sockaddr_in addr;
	int sock = -1;
	
	if(!inet_aton(host, &addr.sin_addr)) {
		if(!(hoste = gethostbyname(host))) {
			fprintf(stderr, "gethostbyname() failed\n");
			return -1;
		}
		addr.sin_addr = *(struct in_addr *) hoste->h_addr;
	}

	if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket() failed\n");
		return -1;
	}
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	if(connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		fprintf(stderr, "connect(host %s, port %d) failed\n", host, port);
		return -1;
	}
	return sock;
}

////////////////////////////////////////////////////////////////////////////////

int read_message(int sock, struct Message *buffer) {
	ssize_t rv = 0;
	char* buf = new char[sizeof(struct Message)];
	int sum = 0;
/*
	printf("readmessage start\n");
	while(i < sizeof(struct Message) && rv > 0) {
		if((rv = read(sock, &buf[i], 1)) != -1) {
			printf("byte %d : %02X", i, buf[i]);
			i++;
		}
	}
	
	memcpy(buffer, buf, sizeof(struct Message));
	*/
	
	fd_set fds;
	struct timeval timeout = {1, 0};
	
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	select(sock + 1, &fds, NULL, NULL, &timeout);
	if(!FD_ISSET(sock, &fds))
		return 0;
	
	// FIXME: correct way would be to count offset
	while(sum < sizeof(struct Message)) {
		if((rv = read(sock, buffer, sizeof(struct Message))) > 0)
			sum += rv;
		else break;
		//if(rv == 0)
		//	return 0;
		if(sum < sizeof(struct Message))
			printf("**read_message problem**");
	}

	//printf("readmessage end\n");
	return (rv > 0);
}

////////////////////////////////////////////////////////////////////////////////

int create_host(int port) {
    int s, c;
    socklen_t addr_len;
    struct sockaddr_in addr;

    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        perror("socket() failed");
        return -1;
    }

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("bind() failed");
        return -2;
    }

    if (listen(s, 3) == -1)
    {
        perror("listen() failed");
        return -3;
    }

    return s;
}
