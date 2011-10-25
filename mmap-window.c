#include "pstore/mmap-window.h"

#include "pstore/compat.h"
#include "pstore/core.h"
#include "pstore/die.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

static struct mmap_window *mmap_window_new(unsigned long max_window_len)
{
	struct mmap_window *self = calloc(sizeof(*self), 1);

	if (!self)
		die("out of memory");

	self->max_window_len	= max_window_len;

	return self;
}

static void mmap_window_delete(struct mmap_window *self)
{
	free(self);
}

static void *mmap_window_end(struct mmap_window *self)
{
	return self->mmap + self->mmap_len;
}

static void mmap_window_mmap(struct mmap_window *self, off_t offset, size_t length)
{
	if (length > self->max_window_len)
		length = self->max_window_len;

	self->mmap_pos	= offset & ~PAGE_MASK;
	self->mmap_len	= length + self->mmap_pos;

	self->mmap = mmap(NULL, self->mmap_len, PROT_READ, MAP_PRIVATE, self->fd, offset & PAGE_MASK);
	if (self->mmap == MAP_FAILED)
		die("mmap");

	self->pos	= offset - self->start_off;

	if (posix_madvise(self->mmap, self->mmap_len, POSIX_MADV_SEQUENTIAL) < 0)
		die("posix_madvise");

	self->mmap_end	= mmap_window_end(self);
}

/*
 * The minimum mmap sliding window size is two pages. That's because when we
 * move the sliding window we need to align the starting offset at page
 * boundary.
 */
#define MIN_MMAP_WINDOW_LEN	(PAGE_SIZE * 2)

struct mmap_window *mmap_window_map(uint64_t max_window_len, int fd, off_t offset, off_t length)
{
	struct mmap_window *self;

	if (max_window_len < MIN_MMAP_WINDOW_LEN)
		max_window_len = MIN_MMAP_WINDOW_LEN;

	self		= mmap_window_new(max_window_len);

	self->fd	= fd;
	self->start_off	= offset;
	self->length	= length;

	mmap_window_mmap(self, offset, length);

	return self;
}

void mmap_window_unmap(struct mmap_window *self)
{
	if (munmap(self->mmap, self->mmap_len) < 0)
		die("munmap");

	mmap_window_delete(self);
}

void *mmap_window_start(struct mmap_window *self)
{
	return self->mmap + self->mmap_pos;
}

void *mmap_window_slide(struct mmap_window *self, void *p)
{
	int64_t remaining;
	uint64_t pos;

	pos = mmap_window_pos_in_region(self, p);

	remaining = self->length - pos;
	if (remaining <= 0)
		die("no remaining data");

	if (munmap(self->mmap, self->mmap_len) < 0)
		die("munmap");

	mmap_window_mmap(self, self->start_off + pos, remaining);

	return mmap_window_start(self);
}
