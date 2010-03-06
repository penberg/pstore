#include "pstore/mmap-window.h"
#include "pstore/compat.h"
#include "pstore/die.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define PAGE_SIZE		getpagesize()
#define PAGE_MASK		(~(PAGE_SIZE-1))

static struct mmap_window *mmap_window__new(unsigned long max_window_len)
{
	struct mmap_window *self = calloc(sizeof(*self), 1);

	if (!self)
		die("out of memory");

	/*
	 * The minimum mmap sliding window size is two pages. That's because
	 * when we move the sliding window we need to align the starting offset
	 * at page boundary.
	 */
	if (max_window_len < PAGE_SIZE * 2)
		die("window too small");

	self->max_window_len	= max_window_len;

	return self;
}

static void mmap_window__delete(struct mmap_window *self)
{
	free(self);
}

static void *mmap_window__end(struct mmap_window *self)
{
	return self->mmap + self->mmap_len;
}

static void mmap_window__mmap(struct mmap_window *self, off_t offset, size_t length)
{
	self->mmap_pos	= offset & ~PAGE_MASK;

	self->mmap_len	= length + self->mmap_pos;
	if (self->mmap_len > self->max_window_len)
		self->mmap_len = self->max_window_len;

	self->mmap = mmap(NULL, self->mmap_len, PROT_READ, MAP_PRIVATE, self->fd, offset & PAGE_MASK);
	if (self->mmap == MAP_FAILED)
		die("mmap");

	self->pos	= offset - self->start_off;

	if (posix_madvise(self->mmap, self->mmap_len, POSIX_MADV_SEQUENTIAL) < 0)
		die("posix_madvise");

	self->mmap_end	= mmap_window__end(self);
}

struct mmap_window *mmap_window__map(uint64_t max_window_len, int fd, off_t offset, off_t length)
{
	struct mmap_window *self = mmap_window__new(max_window_len);

	self->fd	= fd;
	self->start_off	= offset;
	self->length	= length;

	mmap_window__mmap(self, offset, length);

	return self;
}

void mmap_window__unmap(struct mmap_window *self)
{
	if (munmap(self->mmap, self->mmap_len) < 0)
		die("munmap");

	mmap_window__delete(self);
}

void *mmap_window__start(struct mmap_window *self)
{
	return self->mmap + self->mmap_pos;
}

void *mmap_window__slide(struct mmap_window *self, void *p)
{
	int64_t remaining;
	uint64_t pos;

	pos = mmap_window__pos_in_region(self, p);

	remaining = self->length - pos;
	if (remaining <= 0)
		die("no remaining data");

	if (munmap(self->mmap, self->mmap_len) < 0)
		die("munmap");

	mmap_window__mmap(self, self->start_off + pos, remaining);

	return mmap_window__start(self);
}
