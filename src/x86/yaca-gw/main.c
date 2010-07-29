#include "config.h"
#include "socket.h"
#include "serial.h"
#include "hostlist.h"
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
#include <signal.h>
#include <assert.h>

struct Message {
        uint8_t info;
        uint32_t id;
        uint8_t rtr;
        uint8_t length;
        uint8_t data[8];
} __attribute__((__packed__));


int sock, uart;


int get_sender(fd_set *fds) {
    int i = 0;

    while(!FD_ISSET(i, fds))
        i++;

    return i;
}

void send_to_all(struct list_type *list, const char *buffer, int size, int fd_except) {
	struct list_entry *le;

	for(le = list->data; le; le = le->next)
		if(le->fd != fd_except)
			write(le->fd, buffer, size);
}

#define bytewise(var, b) (((unsigned char*)&(var))[b])

void put_buffer(const char *str, const char *buffer, int size) {
   int i, n;

   printf("%s(%d)", str, size);

   for(i = 0; i < size; i++) {
      n = *((unsigned char *)(buffer++));
//      printf("[%d]", n);
      printf("%02X", n);
   }
   printf("\n");
}

#define CMD_SEND    0x03
// buffer max size = sizeof(struct Message)
int create_protocol_transmit(char *tbuffer, const char *buffer) {
	int oi = 0, i;
	struct Message *msg = (struct Message *)buffer;
	
	if(conf.debug) {
		printf("Outgoing CAN Message(rtr: %d): ID %d, data: ", (msg->rtr ? 1 : 0), msg->id);
		put_buffer("", (const char*)(&(msg->data)), msg->length);
	}

	tbuffer[oi++] = CMD_SEND;

	for(i = 0; i < sizeof(struct Message); i++) {
		tbuffer[oi++] = buffer[i];
		if(buffer[i] == 0x55)
			tbuffer[oi++] = 0x01;
	}

	tbuffer[oi++] = 0x55; // line end
	tbuffer[oi++] = 0x00;
	assert(oi < 2 * sizeof(struct Message));

	return oi;
}

void sigterm_handler(int signal) {
	printf("SIGTERM received, exiting.\n");
	socket_close(sock);
	serial_close(uart);
	exit(0);
}

int main(int argc, char **argv) {
	struct sockaddr_in server;
	int pid, client, max, len, tlen, pos = 0, jam = 0;
	int status = 0, i;
	fd_set fds;
	struct list_type list;
	struct list_entry *le;
	char buf[sizeof(struct Message) * 10];
	char tbuf[sizeof(buf) * 2];
	char *pbuf;
	struct Message msgbuf_in, temp_msg;

	if(argc < 2) {
		printf("Usage: %s <config file>\n", argv[0]);
		return 0;
	}
	load_conf(argv[1]);
	sock = socket_init();
	uart = serial_init();
	list_init(&list);
	
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

	while(1) {
		FD_ZERO(&fds);
		max = list_fill_set(&fds, &list);
		FD_SET(sock, &fds);
		if(sock > max)
			max = sock;
		FD_SET(uart, &fds);
		if(uart > max)
			max = uart;
		
		select(max + 1, &fds, NULL, NULL, NULL);
		


		if(FD_ISSET(sock, &fds)) { // new connection?
			client = accept(sock, NULL, 0);
			list_append(&list, client);
		}

		// incoming data from socket?
		for(le = list.data; le; le = le->next) {
			if(!FD_ISSET(le->fd, &fds))
				continue;

			// FIXME: this kind of reception works *USUALLY*!!! fix!
			client = get_sender(&fds);
			if((len = read(client, buf, sizeof(buf))) == 0) { // connection closed?
				socket_close(client); // TODO: check if this works out
				list_remove(&list, client);
			} else {
				if(len >= sizeof(struct Message)) {
					if(conf.debug > 1)
						put_buffer("Socket: ", buf, len);
					pbuf = buf;

					while(len > 0) {
						if(len < sizeof(struct Message)) {
							fprintf(stderr, "wrong message size received on TCP: %d\n", len);
							break;
						}
						
						// TARGET addr: 10 is me, myself and i
						// TODO: change '10' to something more sensible
						// TODO: extend to 'start/stop/restart linux'
/*						memcpy(&temp_msg, buf, sizeof(struct Message));
						if(temp_msg.id == 10) {
							len -= sizeof(struct Message);
							printf("DEBUG message received, sending raw data...\n");
							write(uart, temp_msg.data, temp_msg.length);
							tcdrain(uart);
						} else {*/
							tlen = create_protocol_transmit(tbuf, pbuf);
							send_to_all(&list, (const char *) pbuf, sizeof(struct Message), client); // send to all except ourselves
							
							pbuf += sizeof(struct Message);
							len -= sizeof(struct Message);
							
							if(conf.debug > 1)
								put_buffer("Transmitting via UART", tbuf, tlen);
							write(uart, tbuf, tlen);
							tcdrain(uart);
							if(conf.debug > 1)
								printf("flushed.\n");
//						}
					}
				} else {
					fprintf(stderr, "wrong message size received on TCP: %d\n", len);
				}
			}
		}

		if(FD_ISSET(uart, &fds)) { // incoming data from uart?
			len = read(uart, buf, sizeof(buf));
			if(conf.debug > 1)
				put_buffer("UART: ", buf, len);
			for(i = 0; i < len; i++) {
				switch(status) {
				case 0:
					if(buf[i] == 0x00 && conf.debug > 1) // REPLY_SUCCESS
						printf("Reply: success\n");
					else if(buf[i] == 0x02 && conf.debug > 1)
						printf("Reply: TRANSMIT OK\n");
					else if(buf[i] == 0x01) { // REPLY_DATA
						if(conf.debug > 1)
							printf("Incoming data\n");
						status = 1;
						pos = 0;
					} else if(buf[i] == 0x55)
						status = 2;
					break;

				case 1: // receiving data
					if(buf[i] == 0x55)
						status = 2;
					else
						bytewise(msgbuf_in, pos++) = buf[i];
					break;

				case 2: // 0x55 received before
					status = 0;
					if(buf[i] == 0x00) {
						if(conf.debug) {
							printf("Incoming CAN Message(rtr: %d): ID %d, data: ", (msgbuf_in.rtr ? 1 : 0), msgbuf_in.id);
							put_buffer("", (const char*)&msgbuf_in.data, msgbuf_in.length);
						}
						send_to_all(&list, (const char *) &msgbuf_in, sizeof(msgbuf_in), -1);
						pos = 0;
					} else if(buf[i] == 0x01) {
						bytewise(msgbuf_in, pos++) = 0x55;
						status = 1;
					} else if(buf[i] == 0x02 && conf.debug)
						printf("ping reply\n");
					else if(buf[i] == 0x03)
						fprintf(stderr, "MCU: write buffer treshold reached\n");
					else if(buf[i] == 0x04)
						fprintf(stderr, "MCU: read buffer treshold reached\n");
					break;
				}
			}
		}
	}

	// not that we would ever get here
	socket_close(sock);
	serial_close(uart);

	return 0;
}

