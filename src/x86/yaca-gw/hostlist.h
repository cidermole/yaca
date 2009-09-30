#ifndef HOSTLIST_H
#define HOSTLIST_H

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
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

struct list_entry {
    int fd;
    struct list_entry *next;
};

struct list_type {
    struct list_entry *data;
    unsigned count;
};

void list_init(struct list_type *list);
void list_append(struct list_type *list, int fd);
int list_remove(struct list_type *list, int fd);
int list_fill_set(fd_set *fds, struct list_type *list);

#endif /* HOSTLIST_H */

