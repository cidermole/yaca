#include "hostlist.h"
#include <stdio.h>
#include <stdlib.h>

void list_init(struct list_type *list) {
	list->data = NULL;
	list->count = 0;
}

void list_append(struct list_type *list, int fd) {
    struct list_entry *le;

    le = (struct list_entry *)malloc(sizeof(*le));
    if(!le) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    le->fd = fd;
    le->next = list->data;
    list->data = le;
    list->count++;
}

int list_remove(struct list_type *list, int fd) {
    struct list_entry *li, *lst = NULL;

    if (!list->count)
        return 0;

    for (li = list->data; li; li = li->next) {
        if (li->fd == fd)
            break;
        lst = li;
    }

    if (!li)
        return 0;

    if (lst)
        lst->next = li->next;
    else
        list->data = li->next;

    free(li);
    list->count--;

    return 1;
}

int list_fill_set(fd_set *fds, struct list_type *list) {
    int max = 0;
    struct list_entry *le;

    for (le = list->data; le; le = le->next)
    {
        if (le->fd > max)
            max = le->fd;
        FD_SET(le->fd, fds);
    }

    return max;
}

