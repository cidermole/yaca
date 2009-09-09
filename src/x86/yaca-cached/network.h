#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

struct Message {
        uint8_t info;
        uint32_t id;
        uint8_t rtr;
        uint8_t length;
        uint8_t data[8];
} __attribute__((__packed__));

int connect_socket(const char *host, int port);

int read_message(int sock, struct Message *buffer);

#endif /* NETWORK_H */

