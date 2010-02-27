#include "pstore/segment.h"

#include "pstore/disk-format.h"
#include "pstore/column.h"
#include "pstore/extent.h"
#include "pstore/die.h"

#include <stddef.h>
#include <stdlib.h>

static struct pstore_segment *pstore_segment__new(void)
{
	struct pstore_segment *self = calloc(1, sizeof *self);

	if (!self)
		die("out of memory");

	return self;
}

void pstore_segment__delete(struct pstore_segment *self)
{
	pstore_extent__delete(self->mapped_extent);
}

struct pstore_segment *pstore_segment__read(struct pstore_column *column, int fd)
{
	struct pstore_segment *self = pstore_segment__new();

	self->fd		= fd;
	self->parent		= column;
	self->mapped_extent	= pstore_extent__read(column, column->f_offset, self->fd);

	return self;
}

void *pstore_segment__next_value(struct pstore_segment *self)
{
	void *ret;

	ret = pstore_extent__next_value(self->mapped_extent);
	if (ret)
		return ret;

	if (self->mapped_extent->next_extent == PSTORE_LAST_EXTENT)
		return NULL;

	self->mapped_extent	= pstore_extent__read(self->parent, self->mapped_extent->next_extent, self->fd);

	return pstore_extent__next_value(self->mapped_extent);
}
