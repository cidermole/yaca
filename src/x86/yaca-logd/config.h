#ifndef CONFIG_H
#define CONFIG_H

#define LINE_BUFFER 1024

#ifndef BOOL
	#define BOOL int
	#define TRUE (1 == 1)
	#define FALSE 0
#endif

typedef struct {
	char server[LINE_BUFFER];
	int port;
	char logfile_yaca[LINE_BUFFER];
	char logfile_bulk[LINE_BUFFER];
	int bulk_from;
	int bulk_to;
	int nodeid_from;
	int nodeid_to;
	int multi_from;
	int multi_to;
} conf_t;

extern conf_t conf;

void load_conf(const char* file);

#endif /* CONFIG_H */

