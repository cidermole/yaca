#ifndef IHEX_H
#define IHEX_H

#define ROW_WIDTH 16

int ihex_parse(char *buffer, int bufsize, const char *file);
int ihex_write(int *buffer, int bufsize, const char *file);

#endif /* IHEX_H */

