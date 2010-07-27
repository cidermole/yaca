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

void write_message(int sock, unsigned int id, int length, char d0, char d1, char d2, char d3, char d4, char d5, char d6, char d7);

int read_message(int sock, struct Message *buffer);

char *target_flash_page(const char *buffer, int page_size, int tid, int sock);

uint16_t crc16_update(uint16_t crc, uint8_t c);

void target_eeprom_write(uint32_t id, int sock, uint16_t addr, uint8_t data);

int create_host(int port);

#endif /* NETWORK_H */

