#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define compare(a, b) (!strncmp(a, b, strlen(b)))

conf_t conf;
common_conf_t common_conf;

BOOL _readline(char *buffer, int size, FILE *f) {
	char c;
	int i;

	for(i = 0, c = fgetc(f); !feof(f) && c != '\n' && i < size; c = fgetc(f))
		buffer[i++] = c;
	// FIXME: if only one char remains before EOF, it isn't read. check if this is a problem.
	buffer[i] = '\0';

	if(i >= size) {
		fprintf(stderr, "readline buffer overflow\n");
		exit(0);
	}
	return feof(f) ? FALSE : TRUE;
}

void process_key(const char *key, const char *val) {
	if(compare(key, "flash_size")) {
		conf.flash_size = atoi(val);
	} else if(compare(key, "page_size")) {
		conf.page_size = atoi(val);
	} else if(compare(key, "boot_size")) {
		conf.boot_size = atoi(val);
	} else if(compare(key, "mcu")) {
		strcpy(conf.mcu, val);
	}
}

void process_common_key(const char *key, const char *val) {
	if(compare(key, "server")) {
		strcpy(common_conf.server, val);
	} else if(compare(key, "port")) {
		common_conf.port = atoi(val);
	}
}

void load_common_conf(const char *file) {
	FILE *f = fopen(file, "r");
	char buffer[LINE_BUFFER];
	char *key, *val;
	int i, line = 0;

	// default values
	strcpy(common_conf.server, "192.168.1.1");
	common_conf.port = 1222;
	
	if(!f) {
		fprintf(stderr, "can't open config file \"%s\"\n", file);
		return;
	}

	while(_readline(buffer, LINE_BUFFER, f)) {
		if(buffer[0] == '#' || !strlen(buffer)) // skip #comments and empty lines
			continue;

		for(i = 0; i < strlen(buffer) && buffer[i] != '='; i++);
		if(i != strlen(buffer)) {
			buffer[i] = '\0';
			key = buffer;
			val = &buffer[i+1];
			process_common_key(key, val);
		} else {
			fprintf(stderr, "line %d of %s is not a key (format: key=val)\n", line, file);
			exit(0);
		}
		line++;
	}
	fclose(f);
}

void load_conf(const char *file) {
	FILE *f = fopen(file, "r");
	char buffer[LINE_BUFFER];
	char *key, *val;
	int i, line = 0;

	// default values
	strcpy(conf.mcu, "UNDEFINED");
	conf.flash_size = 1; // should hopefully trigger an error if nothing is specified
	conf.page_size = 64;
	conf.boot_size = 2048;
	
	if(!f) {
		fprintf(stderr, "can't open config file \"%s\"\n", file);
		return;
	}

	while(_readline(buffer, LINE_BUFFER, f)) {
		if(buffer[0] == '#' || !strlen(buffer)) // skip #comments and empty lines
			continue;

		for(i = 0; i < strlen(buffer) && buffer[i] != '='; i++);
		if(i != strlen(buffer)) {
			buffer[i] = '\0';
			key = buffer;
			val = &buffer[i+1];
			process_key(key, val);
		} else {
			fprintf(stderr, "line %d of %s is not a key (format: key=val)\n", line, file);
			exit(0);
		}
		line++;
	}
	fclose(f);
}

