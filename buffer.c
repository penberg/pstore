#include "pstore/buffer.h"

#include "pstore/read-write.h"

#include <stdlib.h>

static struct buffer *buffer_do_new(void *data, size_t capacity)
{
	struct buffer *self = calloc(1, sizeof(*self));

	if (!self)
		return NULL;

	self->capacity	= capacity;

	self->data	= data;

	return self;
}

struct buffer *buffer_new(size_t capacity)
{
	struct buffer *self;
	void *data;

	data		= malloc(capacity);

	if (!data)
		return NULL;

	self		= buffer_do_new(data, capacity);

	return self;
}

int buffer_resize(struct buffer *self, size_t capacity)
{
	free(self->data);

	self->data	= malloc(capacity);
	if (!self->data)
		return -1;

	self->capacity	= capacity;

	self->offset	= 0;

	return 0;
}

void buffer_delete(struct buffer *self)
{
	free(self->data);
	free(self);
}

ssize_t buffer_write(struct buffer *self, int fd)
{
	ssize_t count, nr;

	count = buffer_size(self);

	nr = write_in_full(fd, buffer_start(self), count);
	if (nr != count)
		return -1;

	return nr;
}
