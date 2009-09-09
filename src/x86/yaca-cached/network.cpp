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
	int sum = 0;
	ssize_t rv = 0;

	while(sum < sizeof(struct Message) && rv != -1) {
		if((rv = read(sock, buffer, sizeof(struct Message))) != -1)
			sum += rv;
	}

	return (rv != -1);
}

