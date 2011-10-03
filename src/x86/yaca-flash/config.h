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
	int flash_size;
	int page_size;
	int boot_size;
} common_conf_t;

typedef struct {
	char mcu[LINE_BUFFER];
	int flash_size;
	int page_size;
	int boot_size;
} conf_t;

extern common_conf_t common_conf;
extern conf_t conf;

void load_common_conf(const char* file);
void load_conf(const char* file);

#endif /* CONFIG_H */

