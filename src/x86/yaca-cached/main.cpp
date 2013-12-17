#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"
#include "network.h"
#include "../yaca-path.h"

using namespace std;

#define MAX_FAILS   3 // max. number of timeouts after which no waiting is performed

#define BUFFERS_MAX 1000

// message.info bits (flags)
#define INFO_AUTOINFO 0x01 // auto-info of state change (not implemented)
#define INFO_FAIL     0x02 // fail reply (multiple timeouts -> MAX_FAILS reached)


struct MessageResponse {
        uint8_t info;
        uint32_t id;
        uint8_t rtr;
        uint8_t length;
        uint8_t data[8];
        time_t timestamp;
} __attribute__((__packed__));

struct Buffer {
	int canid[BUFFERS_MAX];
	unsigned char data[BUFFERS_MAX][8];
	int length[BUFFERS_MAX];
	bool buf_ok[BUFFERS_MAX];
	int fail_count[BUFFERS_MAX];
	time_t timestamp[BUFFERS_MAX];
	int nused;
	
	Buffer(): nused(0) {}
	
	bool listening_for(int id) {
		int i;
		
		for(i = 0; i < nused; i++)
			if(canid[i] == id)
				return true;
		return false;
	}
	
	bool used(int id) {
		int i;
		
		for(i = 0; i < nused; i++)
			if(canid[i] == id)
				return buf_ok[i];
		return false;
	}
	
	int fails(int id) {
		int i;
		
		for(i = 0; i < nused; i++)
			if(canid[i] == id)
				return fail_count[i];
		return 0;
	}
	
	void set(int id, const Message *m, bool ok = true) {
		int i, pos = -1;
		
		for(i = 0; i < nused; i++)
			if(canid[i] == id)
				pos = i;
		if(pos == -1) {
			pos = nused++;
			canid[pos] = id;
			fail_count[pos] = ok ? 0 : 1;
			assert(nused < BUFFERS_MAX);
		} else if(ok == false) {
			fail_count[pos]++;
		} else { // pos != -1 && ok == true
			fail_count[pos] = 0;
		}
		memcpy(data[pos], m->data, 8);
		length[pos] = m->length;
		buf_ok[pos] = ok;
		timestamp[pos] = ok ? time(NULL) : 0;
	}
	
	void get(MessageResponse *m, int id) {
		int i, pos = -1;
		
		for(i = 0; i < nused; i++)
			if(canid[i] == id)
				pos = i;
		assert(pos != -1 && buf_ok[pos]);
		memcpy(m->data, data[pos], 8);
		m->length = length[pos];
		m->timestamp = timestamp[pos];
	}
};


int my_max(int a, int b, int c) {
	return ((a > b) ? (a > c ? a : c) : (b > c ? b : c));
}

void handle_message(Buffer *buffer, Message *message) {
	if(!message->rtr && buffer->listening_for(message->id))
		buffer->set(message->id, message);
}

int main(int argc, char **argv) {
	int sock = 0, qsock = 0, csock = 0, fifo_write = 0, id, pid;
	char config_file[1024];
	string listen_pipe, write_pipe, logfname;
	size_t pos;
	fd_set fds;
	Buffer buffer;
	MessageResponse message;
	struct sockaddr_in server;
	bool fail;
	struct timespec query_start, now;
	
	init_yaca_path();
	sprintf(config_file, "%s/src/x86/yaca-cached/conf/yaca-cached.conf", yaca_path);
	load_conf(config_file);
	
	if((sock = connect_socket(conf.server, conf.port)) == -1) {
		fprintf(stderr, "failed to connect to server %s on port %d\n", conf.server, conf.port);
		return 1;
	}
	
    int s, c;
    socklen_t addr_len;
    struct sockaddr_in addr;

    if((s = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() failed");
        return 1;
    }

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(conf.listen_port);
    addr.sin_family = AF_INET;
    
    addr_len = sizeof(sockaddr_in);

    if(bind(s, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("bind() failed");
		return -2;
    }
	
    if(listen(s, 3) == -1) {
		perror("listen() failed");
		return -3;
    }

	if(conf.debug < 2) {
		pid = fork();
		if(pid < 0) {
			fprintf(stderr, "fork() failed\n");
			return 1;
		} else if(pid > 0) { // parent
			return 0;
		}
	}
    
    qsock = s;
	
	while(1) {
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		FD_SET(qsock, &fds);
		if(csock)
			FD_SET(csock, &fds);
		
		select(my_max(sock, qsock, csock) + 1, &fds, NULL, NULL, NULL);
		
		if(FD_ISSET(qsock, &fds)) {
			csock = accept(qsock, (struct sockaddr *) &addr, &addr_len);
		}
		if(FD_ISSET(sock, &fds)) {
			// incoming data from socket, check if the value is buffered and needs to be updated
			if(!read_message(sock, (Message *) &message)) {
				fprintf(stderr, "read_message() from yaca-serial socket failed, exiting...\n");
				return 1;
			}
			
			handle_message(&buffer, (Message *) &message);
		}
		if(FD_ISSET(csock, &fds)) {
			// incoming data from fifo
			if(!read_message(csock, (Message *) &message)) {
				close(csock);
				csock = 0;
				continue;
			}
			if(message.rtr) { // this is a query
				fail = false;
				if(buffer.used(message.id)) {
					buffer.get(&message, message.id);
					message.info = 0; // reply
				} else if(buffer.fails(message.id) >= MAX_FAILS) { // too many timeouts?
					message.info = INFO_FAIL;
					message.timestamp = 0;
					// variable 'fail' is still false -> actually write fail message
				} else {
					// no status info available, query to CAN
					clock_gettime(CLOCK_REALTIME, &query_start);

					message.rtr = 1;
					id = message.id;
					
					write(sock, &message, sizeof(Message));
					fail = false;
					while(message.id != id || message.rtr) {
						if(!read_message(sock, (Message *) &message)) {
							fail = true;
							break;
						}
						handle_message(&buffer, (Message *) &message);

						clock_gettime(CLOCK_REALTIME, &now);
						if(now.tv_sec > query_start.tv_sec && now.tv_nsec > query_start.tv_nsec && (message.id != id || message.rtr)) { // 1 sec passed?
							fail = true;
							break;
						}
					}
					message.info = 0; // reply
					message.timestamp = fail ? 0 : time(NULL);
				}
				message.rtr = 0;

				if(!fail)
					write(csock, &message, sizeof(MessageResponse));
				else
					buffer.set(id, (Message *) &message, false); // not OK (set query to listen for later messages on this)
			} else {
				write(sock, &message, sizeof(Message));
			}
		}
	}
	
	close(sock);
	close(qsock);
	if(csock)
		close(csock);
	return 0;
}

