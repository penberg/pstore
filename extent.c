#include "pstore/extent.h"

#include "pstore/disk-format.h"
#include "pstore/read-write.h"
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

#define MMAP_WINDOW_LEN		(128LL * 1024LL * 1024LL)	/* 128 MiB */

struct pstore_extent *pstore_extent__read(struct pstore_column *column, int fd)
{
	struct pstore_file_extent f_extent;
	struct pstore_extent *self;

	self = pstore_extent__new();

	seek_or_die(fd, column->f_offset, SEEK_SET);
	read_or_die(fd, &f_extent, sizeof(f_extent));

	self->mmap = mmap_window__map(MMAP_WINDOW_LEN, fd, column->f_offset + sizeof(f_extent), f_extent.size);

	self->start = mmap_window__start(self->mmap);

	return self;
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
