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


int my_max(int a, int b) {
	return ((a > b) ? a : b);
}

void handle_message(int fifo, Buffer *buffer, Message *message) {
	if(!message->rtr && buffer->used(message->id)) {
		buffer->set(message->id, message);
		message->info = 1; // auto-info of state change
		write(fifo, message, sizeof(Message));
	}
}

// message.info = 1 -> auto-info of state change
// message.info = 0 -> reply to a query

int main(int argc, char **argv) {
	int sock = 0, fifo = 0, id;
	char config_file[1024];
	string listen_pipe, logfname;
	size_t pos;
	fd_set fds;
	Buffer buffer;
	Message message;

	init_yaca_path();
	sprintf(config_file, "%s/src/x86/yaca-cached/conf/yaca-cached.conf", yaca_path);
	load_conf(config_file);
	listen_pipe = conf.listen_pipe;
	if((pos = listen_pipe.find("$(YACA_PATH)")) != string::npos) {
		listen_pipe.replace(pos, strlen("$(YACA_PATH)"), yaca_path);
	}
	logfname = conf.logfile;
	if((pos = logfname.find("$(YACA_PATH)")) != string::npos) {
		logfname.replace(pos, strlen("$(YACA_PATH)"), yaca_path);
	}
	
	if((sock = connect_socket(conf.server, conf.port)) == -1) {
		fprintf(stderr, "failed to connect to server %s on port %d\n", conf.server, conf.port);
		return 1;
	}
	unlink(listen_pipe.c_str());
	if(mkfifo(listen_pipe.c_str(), 0666) == -1) { // S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH
		fprintf(stderr, "failed to create pipe %s: mkfifo() failed with errno=%d\n", listen_pipe.c_str(), errno);
		return 1;
	}
	if((fifo = open(listen_pipe.c_str(), O_RDWR)) == -1) {
		fprintf(stderr, "failed to open pipe %s: mkfifo() failed with errno=%d\n", listen_pipe.c_str(), errno);
		return 1;
	}
	
	while(1) {
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		FD_SET(fifo, &fds);
		
		select(my_max(sock, fifo) + 1, &fds, NULL, NULL, NULL);
		
		if(FD_ISSET(sock, &fds)) {
			// incoming data from socket, check if the value is buffered and needs to be updated
			read_message(sock, &message);
			handle_message(fifo, &buffer, &message);
		}
		if(FD_ISSET(fifo, &fds)) {
			// incoming data from fifo, this is a query
			read_message(fifo, &message);
			if(buffer.used(message.id)) {
				buffer.get(&message, message.id);
				message.rtr = 0;
				message.info = 0; // reply
				write(fifo, &message, sizeof(Message));
			} else {
				// no status info available, query to CAN
				message.rtr = 1;
				id = message.id;
				write(sock, &message, sizeof(Message));
				while(message.id != id || message.rtr) {
					read_message(sock, &message);
					handle_message(fifo, &buffer, &message);
				}
				buffer.set(message.id, &message);
			}

		}
	}
	
	close(fifo);
	close(sock);
	return 0;
}

