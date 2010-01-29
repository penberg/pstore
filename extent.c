#include "pstore/extent.h"
#include "pstore/die.h"

#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

struct pstore_extent *pstore_extent__new(void)
{
	struct pstore_extent *self = calloc(sizeof *self, 1);

	if (!self)
		die("out of memory");

	return self;
}

void pstore_extent__delete(struct pstore_extent *self)
{
	mmap_window__unmap(self->mmap);

	free(self);
}

void *pstore_extent__next_value(struct pstore_extent *self)
{
	char *start, *end;

restart:
	start = end = self->start;
	do {
		if (mmap_window__in_window(self->mmap, end))
			continue;

		if (!mmap_window__in_region(self->mmap, end))
			return NULL;

		self->start = mmap_window__slide(self->mmap, start);
		goto restart;
	} while (*end++);
	self->start = end;

	return start;
}
