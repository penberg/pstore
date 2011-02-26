#ifndef PSTORE_BUFFER_H
#define PSTORE_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

struct buffer {
	void			*data;
	size_t			offset;
	size_t			capacity;
};

struct buffer *buffer__new(void *data, size_t capacity);
struct buffer *buffer__new_malloc(size_t capacity);
void buffer__delete(struct buffer *self);
void buffer__write(struct buffer *self, int fd);

static inline void *buffer__start(struct buffer *self)
{
	return self->data;
}

static inline size_t buffer__size(struct buffer *self)
{
	return self->offset;
}

static inline bool buffer__has_room(struct buffer *self, size_t len)
{
	return buffer__size(self) + len < self->capacity;
}

static inline bool buffer__in_region(struct buffer *self, void *p)
{
	return p < buffer__start(self) + buffer__size(self);
}

static inline void buffer__clear(struct buffer *self)
{
	self->offset = 0;
}

static inline void buffer__append(struct buffer *self, void *src, size_t len)
{
	memcpy(self->data + self->offset, src, len);
	self->offset	+= len;
}

static inline void buffer__append_char(struct buffer *self, char ch)
{
	char *p = self->data + self->offset++;

	*p = ch;
}

#endif /* PSTORE_BUFFER_H */
