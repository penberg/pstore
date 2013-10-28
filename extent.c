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

#include <inttypes.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MMAP_WINDOW_LEN		MiB(128)

static void *pstore_extent_mmap(struct pstore_extent *self, int fd, off_t offset)
{
	self->mmap	= mmap_window_map(MMAP_WINDOW_LEN, fd, offset + sizeof(struct pstore_file_extent), self->lsize);

	return mmap_window_start(self->mmap);
}

static void *pstore_extent_mmap_next_value(struct pstore_extent *self)
{
	void *start, *end;

restart:
	start = end = self->start;
	for (;;) {
		if (mmap_window_in_window(self->mmap, end + sizeof(unsigned int))) {
			unsigned int *v = end;

			if (!has_zero_byte(*v)) {
				end += sizeof(unsigned int);
				continue;
			}
		}
		break;
	}

	for (;;) {
		unsigned char *c;

		if (!mmap_window_in_window(self->mmap, end))
			goto mmap_slide;

		c = end++;
		if (!*c)
			break;
	}
	self->start = end;

	return start;

mmap_slide:
	if (!mmap_window_in_region(self->mmap, end))
		return NULL;

	self->start = mmap_window_slide(self->mmap, start);
	goto restart;
}

static int pstore_extent_mmap_flush(struct pstore_extent *self, int fd)
{
	buffer_write(self->write_buffer, fd);

	return 0;
}

static int pstore_extent_mmap_prepare_write(struct pstore_extent *self, int fd)
{
	if (self->start_off != 0) {
		if (lseek(fd, self->start_off + self->lsize, SEEK_SET) < 0)
			return -1;
	}

	return 0;
}

static int pstore_extent_mmap_finish_write(struct pstore_extent *self, int fd)
{
	off_t end_off	= lseek(fd, 0, SEEK_CUR);

	if (end_off < 0)
		return -1;

	self->lsize	= end_off - self->start_off;

	return 0;
}

static const struct pstore_extent_ops extent_uncomp_ops = {
	.read		= pstore_extent_mmap,
	.next_value	= pstore_extent_mmap_next_value,
	.flush		= pstore_extent_mmap_flush,
	.prepare_write	= pstore_extent_mmap_prepare_write,
	.finish_write	= pstore_extent_mmap_finish_write,
};

static const struct pstore_extent_ops *extent_ops_table[NR_PSTORE_COMP] = {
	[PSTORE_COMP_NONE]	= &extent_uncomp_ops,
	[PSTORE_COMP_FASTLZ]	= &extent_fastlz_ops,
#ifdef CONFIG_HAVE_SNAPPY
	[PSTORE_COMP_SNAPPY]	= &extent_snappy_ops,
#endif
};

struct pstore_extent *pstore_extent_new(struct pstore_column *parent, uint8_t comp)
{
	struct pstore_extent *self = calloc(sizeof *self, 1);

	if (!self)
		return NULL;

	self->parent		= parent;
	self->comp		= comp;
	self->ops		= extent_ops_table[self->comp];

	return self;
}

void pstore_extent_delete(struct pstore_extent *self)
{
	if (self->mmap)
		mmap_window_unmap(self->mmap);

	if (self->write_buffer)
		buffer_delete(self->write_buffer);

	free(self);
}

struct pstore_extent *pstore_extent_read(struct pstore_column *column, off_t offset, int fd)
{
	struct pstore_file_extent f_extent;
	struct pstore_extent *self;
	off_t off;

	if (lseek(fd, offset, SEEK_SET) < 0)
		return NULL;

	if (read_in_full(fd, &f_extent, sizeof(f_extent)) != sizeof(f_extent))
		return NULL;

	self = pstore_extent_new(column, f_extent.comp);

	self->lsize		= f_extent.lsize;
	self->psize		= f_extent.psize;
	self->next_extent	= f_extent.next_extent;

	if (self->comp >= NR_PSTORE_COMP)
		return NULL;

	self->start		= self->ops->read(self, fd, offset);

	off = lseek(fd, 0, SEEK_CUR);
	if (off < 0)
		return NULL;

	self->start_off		= off;
	self->end_off		= self->start_off + self->psize;

	return self;
}

int pstore_extent_prepare_write(struct pstore_extent *self, int fd, uint64_t max_extent_len)
{
	self->write_buffer	= buffer_new(max_extent_len);

	return 0;
}

int pstore_extent_prepare_append(struct pstore_extent *self)
{
	if (self->comp != PSTORE_COMP_NONE)
		return -EINVAL;

	self->write_buffer	= buffer_new(self->psize - self->lsize);
	if (!self->write_buffer)
		return -ENOMEM;

	return 0;
}

int pstore_extent_flush_write(struct pstore_extent *self, int fd)
{
	off_t off;

	if (self->parent->first_extent == 0) {
		off = lseek(fd, 0, SEEK_CUR);
		if (off < 0)
			return -1;

		self->parent->first_extent = off;
	}

	if (self->ops->prepare_write)
		self->ops->prepare_write(self, fd);

	if (self->start_off == 0) {
		off = lseek(fd, sizeof(struct pstore_file_extent), SEEK_CUR);
		if (off < 0)
			return -1;

		self->start_off = off;
	}

	if (buffer_size(self->write_buffer) > 0)
		self->ops->flush(self, fd);

	if (self->end_off == 0) {
		off = lseek(fd, 0, SEEK_CUR);
		if (off < 0)
			return -1;

		self->end_off = off;
	}

	self->psize		= self->end_off - self->start_off;

	if (self->ops->finish_write)
		self->ops->finish_write(self, fd);

	return 0;
}

int pstore_extent_preallocate(struct pstore_extent *self, int fd, uint64_t extent_len)
{
	off_t off;

	if (self->comp != PSTORE_COMP_NONE)
		return -EINVAL;

	if (!extent_len)
		return -EINVAL;

	if (self->parent->first_extent == 0) {
		off = lseek(fd, 0, SEEK_CUR);
		if (off < 0)
			return -1;

		self->parent->first_extent = off;
	}

	off = lseek(fd, sizeof(struct pstore_file_extent), SEEK_CUR);
	if (off < 0)
		return -1;

	self->start_off = off;

	if (lseek(fd, extent_len - 1, SEEK_CUR) < 0)
		return -1;

	if (write_in_full(fd, "\0", 1) != 1)
		return -1;

	off = lseek(fd, 0, SEEK_CUR);
	if (off < 0)
		return -1;

	self->end_off		= off;
	self->psize		= self->end_off - self->start_off;

	return 0;
}

int pstore_extent_write_metadata(struct pstore_extent *self, off_t next_extent, int fd)
{
	struct pstore_file_extent f_extent;
	off_t f_extent_off, offset;

	offset = lseek(fd, 0, SEEK_CUR);
	if (offset < 0)
		return -1;

	f_extent_off = lseek(fd, self->start_off - sizeof(f_extent), SEEK_SET);
	if (f_extent_off < 0)
		return -1;

	f_extent = (struct pstore_file_extent) {
		.psize		= self->psize,
		.lsize		= self->lsize,
		.comp		= self->comp,
		.next_extent	= next_extent,
	};

	if (write_in_full(fd, &f_extent, sizeof(f_extent)) != sizeof(f_extent))
		return -1;

	if (next_extent == PSTORE_LAST_EXTENT)
		self->parent->last_extent = f_extent_off;

	if (lseek(fd, offset, SEEK_SET) < 0)
		return -1;

	return 0;
}

int pstore_extent_write_value(struct pstore_extent *self, struct pstore_value *value, int fd)
{
	if (self->parent->type != VALUE_TYPE_STRING)
		return -EINVAL;

	if (!pstore_extent_has_room(self, value))
		return -ENOSPC;

	pstore_value_write(value, self->write_buffer);

	return 0;
}

bool pstore_extent_has_room(struct pstore_extent *self, struct pstore_value *value)
{
	return buffer_has_room(self->write_buffer, pstore_value_write_length(value));
}
