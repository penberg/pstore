#include "pstore/extent.h"

#include "pstore/disk-format.h"
#include "pstore/mmap-window.h"
#include "pstore/read-write.h"
#include "pstore/compress.h"
#include "pstore/buffer.h"
#include "pstore/column.h"
#include "pstore/value.h"
#include "pstore/bits.h"
#include "pstore/core.h"
#include "pstore/die.h"

#include <inttypes.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#define MMAP_WINDOW_LEN		MiB(128)

static void *pstore_extent__mmap(struct pstore_extent *self, int fd, off_t offset)
{
	self->mmap	= mmap_window__map(MMAP_WINDOW_LEN, fd, offset + sizeof(struct pstore_file_extent), self->lsize);

	return mmap_window__start(self->mmap);
}

static void *pstore_extent__mmap_next_value(struct pstore_extent *self)
{
	void *start, *end;

restart:
	start = end = self->start;
	for (;;) {
		unsigned char *c;

		if (mmap_window__in_window(self->mmap, end + sizeof(unsigned int))) {
			unsigned int *v = end;

			if (!has_zero_byte(*v)) {
				end += sizeof(unsigned int);
				continue;
			}
		}

		if (!mmap_window__in_window(self->mmap, end))
			goto mmap_slide;

		c = end++;
		if (!*c)
			break;
	}
	self->start = end;

	return start;

mmap_slide:
	if (!mmap_window__in_region(self->mmap, end))
		return NULL;

	self->start = mmap_window__slide(self->mmap, start);
	goto restart;
}

static void pstore_extent__mmap_flush(struct pstore_extent *self, int fd)
{
	buffer__write(self->write_buffer, fd);
}

static void pstore_extent__mmap_prepare_write(struct pstore_extent *self, int fd)
{
	if (self->start_off != 0)
		seek_or_die(fd, self->start_off + self->lsize, SEEK_SET);
}

static void pstore_extent__mmap_finish_write(struct pstore_extent *self, int fd)
{
	off_t end_off	= seek_or_die(fd, 0, SEEK_CUR);

	self->lsize	= end_off - self->start_off;
}

static const struct pstore_extent_ops extent_uncomp_ops = {
	.read		= pstore_extent__mmap,
	.next_value	= pstore_extent__mmap_next_value,
	.flush		= pstore_extent__mmap_flush,
	.prepare_write	= pstore_extent__mmap_prepare_write,
	.finish_write	= pstore_extent__mmap_finish_write,
};

static const struct pstore_extent_ops *extent_ops_table[NR_PSTORE_COMP] = {
	[PSTORE_COMP_NONE]	= &extent_uncomp_ops,
	[PSTORE_COMP_FASTLZ]	= &extent_fastlz_ops,
#ifdef CONFIG_HAVE_SNAPPY
	[PSTORE_COMP_SNAPPY]	= &extent_snappy_ops,
#endif
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

	if (self->write_buffer)
		buffer__delete(self->write_buffer);

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

	self->start_off		= seek_or_die(fd, 0, SEEK_CUR);
	self->end_off		= self->start_off + self->psize;

	return self;
}

void pstore_extent__prepare_write(struct pstore_extent *self, int fd, uint64_t max_extent_len)
{
	self->write_buffer	= buffer__new(max_extent_len);
}

void pstore_extent__prepare_append(struct pstore_extent *self)
{
	if (self->comp != PSTORE_COMP_NONE)
		die("unsupported compression %d", self->comp);

	self->write_buffer	= buffer__new(self->psize - self->lsize);
}

void pstore_extent__flush_write(struct pstore_extent *self, int fd)
{
	if (self->parent->first_extent == 0)
		self->parent->first_extent = seek_or_die(fd, 0, SEEK_CUR);

	if (self->ops->prepare_write)
		self->ops->prepare_write(self, fd);

	if (self->start_off == 0)
		self->start_off = seek_or_die(fd, sizeof(struct pstore_file_extent), SEEK_CUR);

	if (buffer__size(self->write_buffer) > 0)
		self->ops->flush(self, fd);

	if (self->end_off == 0)
		self->end_off	= seek_or_die(fd, 0, SEEK_CUR);

	self->psize		= self->end_off - self->start_off;

	if (self->ops->finish_write)
		self->ops->finish_write(self, fd);
}

void pstore_extent__preallocate(struct pstore_extent *self, int fd, uint64_t extent_len)
{
	if (self->comp != PSTORE_COMP_NONE)
		die("unsupported compression %d", self->comp);

	if (extent_len == 0)
		die("invalid extent length %" PRIu64, extent_len);

	if (self->parent->first_extent == 0)
		self->parent->first_extent = seek_or_die(fd, 0, SEEK_CUR);

	self->start_off		= seek_or_die(fd, sizeof(struct pstore_file_extent), SEEK_CUR);

	seek_or_die(fd, extent_len - 1, SEEK_CUR);
	write_or_die(fd, "\0", 1);

	self->end_off		= seek_or_die(fd, 0, SEEK_CUR);
	self->psize		= self->end_off - self->start_off;
}

void pstore_extent__write_metadata(struct pstore_extent *self, off_t next_extent, int fd)
{
	struct pstore_file_extent f_extent;
	off_t f_extent_off, offset;

	offset = seek_or_die(fd, 0, SEEK_CUR);

	f_extent_off = seek_or_die(fd, self->start_off - sizeof(f_extent), SEEK_SET);
	f_extent = (struct pstore_file_extent) {
		.psize		= self->psize,
		.lsize		= self->lsize,
		.comp		= self->comp,
		.next_extent	= next_extent,
	};
	write_or_die(fd, &f_extent, sizeof(f_extent));

	if (next_extent == PSTORE_LAST_EXTENT)
		self->parent->last_extent = f_extent_off;

	seek_or_die(fd, offset, SEEK_SET);
}

void pstore_extent__write_value(struct pstore_extent *self, struct pstore_value *value, int fd)
{
	if (self->parent->type != VALUE_TYPE_STRING)
		die("unknown type");

	if (!pstore_extent__has_room(self, value))
		die("no room in extent buffer");

	buffer__append(self->write_buffer, value->s, value->len);
	buffer__append_char(self->write_buffer, '\0');
}

bool pstore_extent__has_room(struct pstore_extent *self, struct pstore_value *value)
{
	return buffer__has_room(self->write_buffer, value->len + 1);
}
