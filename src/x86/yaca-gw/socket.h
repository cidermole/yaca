#ifndef SOCKET_H
#define SOCKET_H


#define BACKLOG 5 // the maximum length for the queue of pending connections



int socket_init();
void socket_close(int fd);

#endif /* SOCKET_H */

