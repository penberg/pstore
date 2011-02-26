#include "pstore/buffer.h"

#include "pstore/read-write.h"
#include "pstore/die.h"

#include <stdlib.h>

static struct buffer *buffer__do_new(void *data, size_t capacity)
{
	struct buffer *self = calloc(1, sizeof(*self));

	if (!self)
		die("out of memory");

	self->capacity	= capacity;

	self->data	= data;

	return self;
}

struct buffer *buffer__new(size_t capacity)
{
	struct buffer *self;
	void *data;

	data		= malloc(capacity);

	if (!data)
		die("out of memory");

	self		= buffer__do_new(data, capacity);

	return self;
}

void buffer__resize(struct buffer *self, size_t capacity)
{
	free(self->data);

	self->data	= malloc(capacity);
	if (!self->data)
		die("out of memory");

	self->capacity	= capacity;

	self->offset	= 0;
}

void buffer__delete(struct buffer *self)
{
	free(self->data);
	free(self);
}

void buffer__write(struct buffer *self, int fd)
{
	write_or_die(fd, buffer__start(self), buffer__size(self));
}
