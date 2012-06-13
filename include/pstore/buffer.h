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

struct buffer *buffer_new(size_t capacity);
void buffer_resize(struct buffer *self, size_t capacity);
void buffer_delete(struct buffer *self);
void buffer_write(struct buffer *self, int fd);

static inline void *buffer_start(struct buffer *self)
{
	return self->data;
}

static inline size_t buffer_size(struct buffer *self)
{
	return self->offset;
}

static inline bool buffer_has_room(struct buffer *self, size_t len)
{
	return buffer_size(self) + len < self->capacity;
}

static inline bool buffer_in_region(struct buffer *self, void *p)
{
	return p < buffer_start(self) + buffer_size(self);
}

static inline void buffer_clear(struct buffer *self)
{
	self->offset = 0;
}

static inline void buffer_append(struct buffer *self, const void *src, size_t len)
{
	memcpy(self->data + self->offset, src, len);
	self->offset	+= len;
}

static inline void buffer_append_char(struct buffer *self, char ch)
{
	char *p = self->data + self->offset++;

	*p = ch;
}

#endif /* PSTORE_BUFFER_H */
