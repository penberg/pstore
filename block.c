#include "pstore/block.h"
#include "pstore/die.h"

#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

struct pstore_block *pstore_block__new(void)
{
	struct pstore_block *self = calloc(sizeof *self, 1);

	if (!self)
		die("out of memory");

	return self;
}

void pstore_block__delete(struct pstore_block *self)
{
	if (munmap(self->mmap, self->mmap_len) < 0)
		die("munmap");

	free(self);
}

void *pstore_block__next_value(struct pstore_block *self)
{
	char *s;

	if (self->pos >= self->end)
		return NULL;

	s = self->mmap + self->pos;

	self->pos += strlen(s) + 1;

	return s;
}
