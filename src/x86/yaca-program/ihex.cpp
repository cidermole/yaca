#include "ihex.h"
#include <stdio.h>
#include <string.h>


int ihex_write(int *buffer, int bufsize, const char *file) {
	FILE *f = fopen(file, "w");
	int i, j, checksum;
	
	if(!f) {
		fprintf(stderr, "ihex: error: failed to open file %s\n", file);
		return 0;
	}

	for(i = 0; i < bufsize / ROW_WIDTH; i++) {
		fprintf(f, ":%02X%04X00", ROW_WIDTH, i * ROW_WIDTH);
		checksum = ROW_WIDTH + (((i * ROW_WIDTH) >> 8) & 0xff) + ((i * ROW_WIDTH) & 0xff);
		for(j = 0; j < ROW_WIDTH; j++) {
			fprintf(f, "%02X", buffer[i * ROW_WIDTH + j] & 0xff);
			checksum += buffer[i * ROW_WIDTH + j] & 0xff;
		}
		fprintf(f, "%02X\n", ((checksum ^ 0xff) + 1) & 0xff);
	}
	fprintf(f, ":00000001FF");
	fclose(f);
	return 1;
}

int ihex_parse(char *buffer, int bufsize, const char *file) {
	FILE *f = fopen(file, "r");
	char *addr, *highest_addr = buffer;
	int i, length, line = 0, type, byte, sum;

	if(!f) {
		fprintf(stderr, "ihex: error: failed to open file %s\n", file);
		goto ihex_p_err;
	}

	memset(buffer, bufsize, 0);

	while(!feof(f)) {
		/*
		 * all numbers are in hex
		 *
		 * :LLAAAATT[data]CC
		 * :101EF0008F
		 *
		 * L: length: data bytes
		 * A: address offset
		 * T: type (00: data, 01: eof)
		 * C: checksum
		 */
		sum = 0;
		for(i = fgetc(f); (char)i != ':' && (char)i != -1 && !feof(f); i = fgetc(f));
		if((char)i != ':' || feof(f)) {
			fprintf(stderr, "ihex: parse error: ':' at file end\n");
			goto ihex_p_err;
		}

		if(fscanf(f, "%2X%4X%2X", &length, &i, &type) != 3) {
			fprintf(stderr, "ihex: parse error: scan error on line %d\n", line);
			goto ihex_p_err;
		}
		sum += length + type + ((unsigned char)i) + ((unsigned char)(i >> 8));
		if(i < 0 || i > bufsize) {
			fprintf(stderr, "ihex: error: buffer overflow at line %d (%d bytes buffer)\n", line, bufsize);
			goto ihex_p_err;
		}
		addr = &buffer[i];
		
		if(type == 0x01) {
			fclose(f);
			return (highest_addr - buffer);
		} else if(type != 0x00) {
			fprintf(stderr, "ihex: warning: unsupported record type %02X at line %d\n", type, line);
//			goto ihex_p_err;
			continue;
		}

		for(i = 0; i < length; i++) {
			if(fscanf(f, "%2X", &byte) != 1) {
				fprintf(stderr, "ihex: parse error: scan error on line %d\n", line);
				goto ihex_p_err;
			}
			sum += (unsigned char)byte;
			*(addr++) = (char)byte;
		}
		if(fscanf(f, "%2X", &i) != 1) {
			fprintf(stderr, "ihex: parse error: scan error of checksum at line %d\n", line);
			goto ihex_p_err;
		}
		sum += (unsigned char)i;
		if(sum & 0xff) {
			fprintf(stderr, "ihex: checksum error on line %d\n", line);
			goto ihex_p_err;
		}
		if(addr > highest_addr)
			highest_addr = addr;

		line++;
	}
	
	fprintf(stderr, "ihex: parse error: unexpected end of file without 0x01 typed record\n");

ihex_p_err:
	fclose(f);
	return 0;
}

