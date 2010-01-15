#include "pstore/buffer.h"
#include "pstore/die.h"

#include <stdlib.h>

struct buffer *buffer__new(size_t capacity)
{
	struct buffer *self = calloc(1, sizeof(*self));

	if (!self)
		die("out of memory");

	self->capacity	= capacity;

	self->data	= malloc(capacity);

	if (!self->data)
		die("out of memory");

	return self;
}

void buffer__delete(struct buffer *self)
{
	free(self->data);
	free(self);
}
