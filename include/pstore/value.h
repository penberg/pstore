#ifndef PSTORE_VALUE_H
#define PSTORE_VALUE_H

#include "pstore/buffer.h"

#include <stddef.h>

enum value_type {
	VALUE_TYPE_STRING		= 0x01,
};

struct pstore_value {
	const char		*s;
	size_t			len;
};

static inline void pstore_value_write(struct pstore_value *self, struct buffer *buf)
{
	buffer_append(buf, self->s, self->len);
	buffer_append_char(buf, '\0');
}

static inline size_t pstore_value_write_length(struct pstore_value *self)
{
	return self->len + 1;
}

#endif /* PSTORE_VALUE_H */
