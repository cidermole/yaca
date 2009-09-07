#ifndef YACA_PATH_H
#define YACA_PATH_H

#include <stdio.h>
#include <stdlib.h>

char yaca_path[1024];

void init_yaca_path() {
	FILE *f = fopen("/etc/yaca.path", "r");
	size_t data;

	if(!f) {
		fprintf(stderr, "init_yaca_path() failed: can't open /etc/yaca.path\n");
		exit(1);
	}
	data = fread(yaca_path, 1, 1024 - 1, f);
	yaca_path[data] = '\0';
}

#endif /* YACA_PATH_H */

