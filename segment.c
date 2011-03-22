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
	if (self->map_extent)
		pstore_extent__delete(self->map_extent);
	free(self);
}

struct pstore_segment *pstore_segment__read(struct pstore_column *column, int fd)
{
	struct pstore_segment *self = pstore_segment__new();

	self->fd		= fd;
	self->parent		= column;
	self->map_extent	= pstore_extent__read(column, column->first_extent, self->fd);

	return self;
}

struct pstore_extent *pstore_segment__next_extent(struct pstore_segment *self)
{
	struct pstore_extent *ret = self->map_extent;

	if (!ret)
		return NULL;

	if (!pstore_extent__is_last(ret))
		self->map_extent	= pstore_extent__read(self->parent, self->map_extent->next_extent, self->fd);
	else
		self->map_extent	= NULL;

	return ret;
}

void *pstore_segment__next_value(struct pstore_segment *self)
{
	void *ret;

	ret = pstore_extent__next_value(self->map_extent);
	if (ret)
		return ret;

	if (pstore_extent__is_last(self->map_extent))
		return NULL;

	/*
	 * The following kludge is needed temporarily. As 'pstore import --append'
	 * currently just appends a new extent to the segment, empty preallocated
	 * extents can exist in the middle of segments.
	 */
next_extent:
	self->map_extent	= pstore_extent__read(self->parent, self->map_extent->next_extent, self->fd);

	ret = pstore_extent__next_value(self->map_extent);
	if (ret)
		return ret;

	if (pstore_extent__is_last(self->map_extent))
		return NULL;

	goto next_extent;
}
