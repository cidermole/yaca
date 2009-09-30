#include "fifo.h"

void fifo2_init (fifo_t *f, uint8_t *buffer, const uint16_t size)
{
	f->count = 0;
	f->pread = f->pwrite = buffer;
	f->read2end = f->write2end = f->size = size;
}

uint8_t fifo2_put (fifo_t *f, const uint8_t data)
{
	return _inline_fifo2_put (f, data);
}

uint8_t fifo2_get_wait (fifo_t *f)
{
	while (!f->count);

	return _inline_fifo2_get (f);
}

int fifo2_get_nowait (fifo_t *f)
{
	if (!f->count)		return -1;

	return (int) _inline_fifo2_get (f);
}
