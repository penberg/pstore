#include "pstore/extent.h"

#include "pstore/disk-format.h"
#include "pstore/mmap-window.h"
#include "pstore/read-write.h"
#include "pstore/compress.h"
#include "pstore/buffer.h"
#include "pstore/column.h"
#include "pstore/value.h"
#include "pstore/core.h"
#include "pstore/die.h"

#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#define MMAP_WINDOW_LEN		MiB(128)

static void *pstore_extent__mmap(struct pstore_extent *self, int fd, off_t offset)
{
	self->mmap	= mmap_window__map(MMAP_WINDOW_LEN, fd, offset + sizeof(struct pstore_file_extent), self->psize);
	
	return mmap_window__start(self->mmap);
}

static void *pstore_extent__mmap_next_value(struct pstore_extent *self)
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

static void pstore_extent__mmap_flush(struct pstore_extent *self, int fd)
{
	buffer__write(self->buffer, fd);
}

static void pstore_extent__mmap_finish_write(struct pstore_extent *self)
{
	self->lsize		= self->psize;
}

static const struct pstore_extent_ops extent_uncomp_ops = {
	.read		= pstore_extent__mmap,
	.next_value	= pstore_extent__mmap_next_value,
	.flush		= pstore_extent__mmap_flush,
	.finish_write	= pstore_extent__mmap_finish_write,
};

static const struct pstore_extent_ops *extent_ops_table[NR_PSTORE_COMP] = {
	[PSTORE_COMP_NONE]	= &extent_uncomp_ops,
	[PSTORE_COMP_LZO1X_1]	= &extent_lzo1x_1_ops,
	[PSTORE_COMP_FASTLZ]	= &extent_fastlz_ops,
	[PSTORE_COMP_QUICKLZ]	= &extent_quicklz_ops,
};

struct pstore_extent *pstore_extent__new(struct pstore_column *parent, uint8_t comp)
{
	struct pstore_extent *self = calloc(sizeof *self, 1);

	if (!self)
		die("out of memory");

	self->parent		= parent;
	self->comp		= comp;
	self->ops		= extent_ops_table[self->comp];

	return self;
}

void pstore_extent__delete(struct pstore_extent *self)
{
	if (self->mmap)
		mmap_window__unmap(self->mmap);

	if (self->buffer)
		buffer__delete(self->buffer);

	free(self);
}

struct pstore_extent *pstore_extent__read(struct pstore_column *column, off_t offset, int fd)
{
	struct pstore_file_extent f_extent;
	struct pstore_extent *self;

	seek_or_die(fd, offset, SEEK_SET);
	read_or_die(fd, &f_extent, sizeof(f_extent));

	self = pstore_extent__new(column, f_extent.comp);

	self->lsize		= f_extent.lsize;
	self->psize		= f_extent.psize;
	self->next_extent	= f_extent.next_extent;

	if (self->comp >= NR_PSTORE_COMP)
		die("unknown compression %d", self->comp);

	self->start		= self->ops->read(self, fd, offset);

	return self;
}

void pstore_extent__prepare_write(struct pstore_extent *self, int fd, uint64_t max_extent_len)
{
	self->buffer		= buffer__new(max_extent_len);
}

void pstore_extent__flush_write(struct pstore_extent *self, int fd)
{
	if (self->parent->f_offset == 0)
		self->parent->f_offset = seek_or_die(fd, 0, SEEK_CUR);

	self->start_off		= seek_or_die(fd, sizeof(struct pstore_file_extent), SEEK_CUR);

	if (buffer__size(self->buffer) > 0)
		self->ops->flush(self, fd);

	self->end_off		= seek_or_die(fd, 0, SEEK_CUR);
	self->psize		= self->end_off - self->start_off;

	if (self->ops->finish_write)
		self->ops->finish_write(self);
}

void pstore_extent__write_metadata(struct pstore_extent *self, off_t next_extent, int fd)
{
	struct pstore_file_extent f_extent;
	off_t offset;

	offset = seek_or_die(fd, 0, SEEK_CUR);

	seek_or_die(fd, self->start_off - sizeof(f_extent), SEEK_SET);
	f_extent = (struct pstore_file_extent) {
		.psize		= self->psize,
		.lsize		= self->lsize,
		.comp		= self->comp,
		.next_extent	= next_extent,
	};
	write_or_die(fd, &f_extent, sizeof(f_extent));

	seek_or_die(fd, offset, SEEK_SET);
}

void pstore_extent__write_value(struct pstore_extent *self, struct pstore_value *value, int fd)
{
	if (self->parent->type != VALUE_TYPE_STRING)
		die("unknown type");

	if (!pstore_extent__has_room(self, value))
		die("no room in extent buffer");

	buffer__append(self->buffer, value->s, value->len);
	buffer__append_char(self->buffer, '\0');
}

bool pstore_extent__has_room(struct pstore_extent *self, struct pstore_value *value)
{
	return buffer__has_room(self->buffer, value->len + 1);
}
