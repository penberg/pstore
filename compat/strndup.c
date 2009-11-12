#include "pstore/compat.h"

#include <stdlib.h>
#include <string.h>

char *strndup(const char *s, size_t n)
{
	size_t size;
	char *p;

	if (!s)
		return NULL;

	size = strlen(s);
	if (size > n)
		size = n;
	size++;

	p = malloc(size);
	memcpy(p, s, size);
	p[size - 1] = '\0';

	return p;
}
