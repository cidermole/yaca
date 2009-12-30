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
#include <fcntl.h>
#include <errno.h>

#include "config.h"
#include "network.h"
#include "../yaca-path.h"

using namespace std;

#define BUFFERS_MAX 1000

struct Buffer {
	int canid[BUFFERS_MAX];
	unsigned char data[BUFFERS_MAX][8];
	int length[BUFFERS_MAX];
	int nused;
	
	Buffer(): nused(0) {}
	
	bool used(int id) {
		int i;
		
		for(i = 0; i < nused; i++)
			if(canid[i] == id)
				return true;
		return false;
	}
	
	void set(int id, const Message *m) {
		int i, pos = -1;
		
		for(i = 0; i < nused; i++)
			if(canid[i] == id)
				pos = i;
		if(pos == -1) {
			pos = nused++;
			canid[pos] = id;
			assert(nused < BUFFERS_MAX);
		}
		memcpy(data[pos], m->data, 8);
		length[pos] = m->length;
	}
	
	void get(Message *m, int id) {
		int i, pos = -1;
		
		for(i = 0; i < nused; i++)
			if(canid[i] == id)
				pos = i;
		assert(pos != -1);
		memcpy(m->data, data[pos], 8);
		m->length = length[pos];
	}
};


int my_max(int a, int b, int c) {
	return ((a > b) ? (a > c ? a : c) : (b > c ? b : c));
}

void handle_message(Buffer *buffer, Message *message) {
	int fifo_write;
	
	if(!message->rtr) {
		buffer->set(message->id, message);
/*		message->info = 1; // auto-info of state change
		if((fifo_write = open(write_pipe.c_str(), O_WRONLY)) != -1) {
			write(fifo_write, message, sizeof(Message));
			close(fifo_write);
		}*/
		// pipe opened with O_NONBLOCK does not work, this is disabled for now
	}
}

// message.info = 1 -> auto-info of state change
// message.info = 0 -> reply to a query

int main(int argc, char **argv) {
	int sock = 0, qsock = 0, csock = 0, fifo_write = 0, id, pid;
	char config_file[1024];
	string listen_pipe, write_pipe, logfname;
	size_t pos;
	fd_set fds;
	Buffer buffer;
	Message message;
	struct sockaddr_in server;
	bool fail;
	
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
    
	pid = fork();
	if(pid < 0) {
		fprintf(stderr, "fork() failed\n");
		return 1;
	} else if(pid > 0) { // parent
		return 0;
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
			if(!read_message(sock, &message)) {
				fprintf(stderr, "read_message() from yaca-serial socket failed, exiting...\n");
				return 1;
			}
			
			if(!message.rtr && buffer.used(message.id)) {
				//handle_message(&buffer, &message);
				buffer.set(message.id, &message);
			}
		}
		if(FD_ISSET(csock, &fds)) {
			// incoming data from fifo
			if(!read_message(csock, &message)) {
				close(csock);
				csock = 0;
				continue;
			}
			if(message.rtr) { // this is a query
				if(buffer.used(message.id)) {
					buffer.get(&message, message.id);
				} else {
					// no status info available, query to CAN
					message.rtr = 1;
					id = message.id;
					
					write(sock, &message, sizeof(Message));
					fail = false;
					while(message.id != id || message.rtr) {
						if(!poll_message(sock)) {
							fail = true;
							break;
						}
						read_message(sock, &message);
						handle_message(&buffer, &message);
					}
				}
				message.rtr = 0;
				message.info = 0; // reply

				if(!fail)
					write(csock, &message, sizeof(Message));
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

