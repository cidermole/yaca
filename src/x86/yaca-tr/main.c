#include <stdio.h>
#include <stdlib.h>
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

struct Message {
        uint8_t info;
        uint32_t id;
        uint8_t rtr;
        uint8_t length;
        uint8_t data[8];
} __attribute__((__packed__));

int main(int argc, char **argv) {
	struct hostent *host;
	struct sockaddr_in addr;
	int sock = 0;
	
	if(argc < 3) {
		printf("Too few arguments. Usage: %s <id> <rtr 1/0> <data bytes in hex>\n", argv[0]);
		return 0;
	}

	// TODO: conf
	if(!inet_aton("192.168.1.1", &addr.sin_addr)) {
		if(!(host = gethostbyname("192.168.1.1"))) {
			fprintf(stderr, "gethostbyname() failed\n");
			return 1;
		}
		addr.sin_addr = *(struct in_addr*)host->h_addr;
	}

	if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "failed to create socket\n");
		return 1;
	}
	addr.sin_port = htons(1222); // TODO: conf
	addr.sin_family = AF_INET;

	if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "failed to connect()\n");
		return 1;
	}

	int id = atoi(argv[1]);
	int rtr = atoi(argv[2]);
//	int length = atoi(argv[3]);
	int length = 0;
	if(argc > 3)
		length = strlen(argv[3]) / 2;
	if(length > 8)
		length = 8;

	int i, j;
	char *s = argv[3];
	struct Message msg;
	msg.id = id;
	msg.rtr = rtr;
	msg.length = length;

	for(i = 0; i < length; i++) {
		sscanf(s, "%02X", &j);
		msg.data[i] = j;
		s += 2;
	}
	printf("%d (rtr:%d)l:%d ", msg.id, msg.rtr, msg.length);
	for(i = 0; i < msg.length; i++)
		printf("%02X", msg.data[i]);
	printf("\n");
	write(sock, &msg, sizeof(msg));

#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif

	return 0;
}
