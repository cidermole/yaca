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

struct Message {
        uint8_t info;
        uint32_t id;
        uint8_t rtr;
        uint8_t length;
        uint8_t data[8];
} __attribute__((__packed__));

int get_sender(fd_set *fds) {
    int i = 0;

    while(!FD_ISSET(i, fds))
        i++;

    return i;
}

void send_to_all(struct list_type *list, const char *buffer, int size) {
	struct list_entry *le;

	for(le = list->data; le; le = le->next)
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
int create_protocol_transmit(char *tbuffer, char *buffer) {
//	char out[100];
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

//	memcpy(buffer, out, oi);
	return oi;
}

int main(int argc, char **argv) {
	struct sockaddr_in server;
	int sock, client, uart, max, len, tlen, pos = 0, jam = 0;
	int status = 0, i;
	fd_set fds;
	struct list_type list;
	char buf[sizeof(struct Message) * 10];
	char tbuf[100];
	char *pbuf;
<<<<<<< HEAD:src/x86/yaca-gw/main.c
	struct Message msgbuf_in, temp_msg;
=======
	struct Message msgbuf_in;
>>>>>>> d85447ef43edc1c8745fd9135cb6a0c0d5e057b3:src/x86/yaca-gw/main.c

	if(argc < 2) {
		printf("Usage: %s <config file>\n", argv[0]);
		return 0;
	}
	load_conf(argv[1]);
	sock = socket_init();
	uart = serial_init();
	list_init(&list);

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
		} else if(FD_ISSET(uart, &fds)) { // incoming data from uart?
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
						if(conf.debug && msgbuf_in.data[0] == 0xF0 && msgbuf_in.length)
							printf("(debug msg, not forwarding)\n");
						else
							send_to_all(&list, (const char*)&msgbuf_in, sizeof(msgbuf_in));
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
		} else { // incoming data from socket
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
						// TARGET addr: 10 is me, myself and i
						memcpy(&temp_msg, buf, sizeof(struct Message));
						if(temp_msg.id == 10) {
							len -= sizeof(struct Message);
							printf("DEBUG message received, sending raw data...\n");
							write(uart, temp_msg.data, temp_msg.length);
							tcdrain(uart);
						} else {
							tlen = create_protocol_transmit(tbuf, pbuf);
							pbuf += sizeof(struct Message);
							len -= sizeof(struct Message);
							if(conf.debug > 1)
								put_buffer("Transmitting via UART", tbuf, tlen);
							write(uart, tbuf, tlen);
							tcdrain(uart);
							if(conf.debug > 1)
								printf("flushed.\n");
						}
					}
				} else {
					fprintf(stderr, "wrong message size received on TCP: %d\n", len);
				}
			}
		}
	}

	// not that we would ever get here
	socket_close(sock);
	serial_close(uart);

	return 0;
}

