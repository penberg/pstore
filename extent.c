#include "pstore/extent.h"

#include "pstore/disk-format.h"
#include "pstore/mmap-window.h"
#include "pstore/read-write.h"
#include "pstore/column.h"
#include "pstore/buffer.h"
#include "pstore/value.h"
#include "pstore/core.h"
#include "pstore/die.h"

#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

struct pstore_extent *pstore_extent__new(struct pstore_column *parent)
{
	struct pstore_extent *self = calloc(sizeof *self, 1);

	if (!self)
		die("out of memory");

	self->parent		= parent;

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

	self = pstore_extent__new(column);

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

#define WRITEOUT_SIZE		KB(32)

void pstore_extent__prepare_write(struct pstore_extent *self, int fd)
{
	self->buffer		= buffer__new(WRITEOUT_SIZE);
	self->start_off		= seek_or_die(fd, sizeof(struct pstore_file_extent), SEEK_CUR);
}

void pstore_extent__finish_write(struct pstore_extent *self, int fd)
{
	struct pstore_file_extent f_extent;

	if (buffer__size(self->buffer) > 0)
		buffer__write(self->buffer, fd);

	buffer__delete(self->buffer);

	self->end_off		= seek_or_die(fd, 0, SEEK_CUR);
	self->size		= self->end_off - self->start_off;

	seek_or_die(fd, -(sizeof(f_extent) + self->size), SEEK_CUR);
	f_extent = (struct pstore_file_extent) {
		.size	= self->size,
	};
	write_or_die(fd, &f_extent, sizeof(f_extent));

	seek_or_die(fd, self->size, SEEK_CUR);
}

void pstore_extent__write_value(struct pstore_extent *self, struct pstore_value *value, int fd)
{
	if (self->parent->type != VALUE_TYPE_STRING)
		die("unknown type");

	if (!buffer__has_room(self->buffer, value->len + 1)) {
		buffer__write(self->buffer, fd);
		buffer__clear(self->buffer);
	}

	buffer__append(self->buffer, value->s, value->len);
	buffer__append_char(self->buffer, '\0');
}
