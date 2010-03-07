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
	if (self->mmap)
		mmap_window__unmap(self->mmap);

	if (self->buffer)
		buffer__delete(self->buffer);

	free(self);
}

#define MMAP_WINDOW_LEN		MiB(128)

static void *pstore_extent__mmap(struct pstore_extent *self, int fd, off_t offset)
{
	self->mmap	= mmap_window__map(MMAP_WINDOW_LEN, fd, offset + sizeof(struct pstore_file_extent), self->psize);
	
	return mmap_window__start(self->mmap);
}

static const struct pstore_extent_ops extent_uncomp_ops = {
	.read		= pstore_extent__mmap,
};

static const struct pstore_extent_ops extent_lzo1x_1_ops = {
	.read		= pstore_extent__decompress,
};

static const struct pstore_extent_ops *extent_ops_table[NR_PSTORE_COMP] = {
	[PSTORE_COMP_NONE]	= &extent_uncomp_ops,
	[PSTORE_COMP_LZO1X_1]	= &extent_lzo1x_1_ops,
};

struct pstore_extent *pstore_extent__read(struct pstore_column *column, off_t offset, int fd)
{
	struct pstore_file_extent f_extent;
	struct pstore_extent *self;

	self = pstore_extent__new(column);

	seek_or_die(fd, offset, SEEK_SET);
	read_or_die(fd, &f_extent, sizeof(f_extent));

	self->lsize		= f_extent.lsize;
	self->psize		= f_extent.psize;
	self->comp		= f_extent.comp;
	self->next_extent	= f_extent.next_extent;

	if (self->comp >= NR_PSTORE_COMP)
		die("unknown compression %d", self->comp);

	self->ops		= extent_ops_table[self->comp];

	self->start		= self->ops->read(self, fd, offset);

	return self;
}

static void *pstore_extent__buffer_next_value(struct pstore_extent *self)
{
	char *start, *end;

	start = end = self->start;
	do {
		if (buffer__in_region(self->buffer, end))
			continue;

		return NULL;
	} while (*end++);
	self->start = end;

	return start;
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

void *pstore_extent__next_value(struct pstore_extent *self)
{
	void *ret;

	switch (self->comp) {
	case PSTORE_COMP_LZO1X_1:
		ret = pstore_extent__buffer_next_value(self);
		break;
	case PSTORE_COMP_NONE:
		ret = pstore_extent__mmap_next_value(self);
		break;
	default:
		die("unknown compression %d", self->comp);
	};

	return ret;
}

void pstore_extent__prepare_write(struct pstore_extent *self, int fd, uint64_t max_extent_len)
{
	self->buffer		= buffer__new(max_extent_len);
	self->start_off		= seek_or_die(fd, sizeof(struct pstore_file_extent), SEEK_CUR);
}

static void pstore_extent__do_flush(struct pstore_extent *self, int fd)
{
	switch (self->comp) {
	case PSTORE_COMP_LZO1X_1: {
		pstore_extent__compress(self, fd);
		break;
	}
	case PSTORE_COMP_NONE:
		buffer__write(self->buffer, fd);
		break;
	default:
		die("unknown compression %d", self->comp);
	};
}

void pstore_extent__flush_write(struct pstore_extent *self, int fd)
{
	if (buffer__size(self->buffer) > 0)
		pstore_extent__do_flush(self, fd);

	buffer__delete(self->buffer);
}

void pstore_extent__finish_write(struct pstore_extent *self, off_t next_extent, int fd)
{
	struct pstore_file_extent f_extent;

	self->end_off		= seek_or_die(fd, 0, SEEK_CUR);
	self->psize		= self->end_off - self->start_off;

	if (self->comp == PSTORE_COMP_NONE)
		self->lsize		= self->psize;

	seek_or_die(fd, -(sizeof(f_extent) + self->psize), SEEK_CUR);
	f_extent = (struct pstore_file_extent) {
		.psize		= self->psize,
		.lsize		= self->lsize,
		.comp		= self->comp,
		.next_extent	= next_extent,
	};
	write_or_die(fd, &f_extent, sizeof(f_extent));

	seek_or_die(fd, self->psize, SEEK_CUR);
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
