#include "pstore/buffer.h"

#include "pstore/read-write.h"
#include "pstore/die.h"

#include <stdlib.h>

static struct buffer *buffer_do_new(void *data, size_t capacity)
{
    struct buffer *self = calloc(1, sizeof(*self));

    if (!self)
        die("out of memory");

    self->capacity    = capacity;

    self->data    = data;

    return self;
}

struct buffer *buffer_new(size_t capacity)
{
    struct buffer *self;
    void *data;

    data        = malloc(capacity);

    if (!data)
        die("out of memory");

    self        = buffer_do_new(data, capacity);

    return self;
}

void buffer_resize(struct buffer *self, size_t capacity)
{
    free(self->data);

    self->data    = malloc(capacity);
    if (!self->data)
        die("out of memory");

    self->capacity    = capacity;

    self->offset    = 0;
}

void buffer_delete(struct buffer *self)
{
    free(self->data);
    free(self);
}

void buffer_write(struct buffer *self, int fd)
{
    write_or_die(fd, buffer_start(self), buffer_size(self));
}
