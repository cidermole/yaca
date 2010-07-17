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
	int tcp_in_id;
	int tcp_out_id;
	char pppd_params[LINE_BUFFER];
} conf_t;

extern conf_t conf;

void load_conf(const char* file);

#endif /* CONFIG_H */

