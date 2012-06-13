#ifndef PSTORE_VALUE_H
#define PSTORE_VALUE_H

#include <stddef.h>

enum value_type {
	VALUE_TYPE_STRING		= 0x01,
};

struct pstore_value {
	const char		*s;
	size_t			len;
};

#endif /* PSTORE_VALUE_H */
