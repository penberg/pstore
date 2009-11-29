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

/*
 * The minimum mmap sliding window size is two pages. That's because when we
 * move the sliding window we need to align the starting offset at page
 * boundary.
 */
#define MMAP_WINDOW_LEN		(128LL * 1024LL * 1024LL)	/* 128 MiB */

static struct mmap_window *mmap_window__new(void)
{
	struct mmap_window *self = calloc(sizeof(*self), 1);

	if (!self)
		die("out of memory");

	return self;
}

static void mmap_window__delete(struct mmap_window *self)
{
	free(self);
}

static void *mmap_window__end(struct mmap_window *self)
{
	return self->mmap + self->mmap_pos + self->mmap_len;
}

struct mmap_window *mmap_window__map(int fd, off64_t offset, off64_t length)
{
	struct mmap_window *self = mmap_window__new();
	size_t mmap_len;

	mmap_len = length;
	if (mmap_len > MMAP_WINDOW_LEN)
		mmap_len = MMAP_WINDOW_LEN;

	self->mmap = mmap(NULL, mmap_len, PROT_READ, MAP_PRIVATE, fd, offset & PAGE_MASK);
	if (self->mmap == MAP_FAILED)
		die("mmap");

	self->mmap_pos	= offset & ~PAGE_MASK;
	self->mmap_len	= mmap_len;
	self->length	= length;
	self->offset	= offset & PAGE_MASK;
	self->fd	= fd;

	if (posix_madvise(self->mmap, self->mmap_len, POSIX_MADV_SEQUENTIAL) < 0)
		die("posix_madvise");

	self->mmap_end	= mmap_window__end(self);

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
	off64_t window_start;
	size_t remaining;
	size_t mmap_pos;
	uint64_t pos;

	mmap_pos = mmap_window__ptr_pos(self, p);

	pos = self->pos + mmap_pos; 

	remaining = self->length - pos - 1;
	if (remaining == 0)
		return NULL;

	if (munmap(self->mmap, self->mmap_len) < 0)
		die("munmap");

	self->mmap_len = MMAP_WINDOW_LEN;
	if (self->mmap_len > remaining)
		self->mmap_len = remaining;

	window_start = pos & PAGE_MASK;

	self->mmap = mmap(NULL, self->mmap_len, PROT_READ, MAP_PRIVATE, self->fd, self->offset + window_start);
	if (self->mmap == MAP_FAILED)
		die("mmap");

	self->mmap_pos	= pos - window_start;
	self->pos	= pos;

	if (posix_madvise(self->mmap, self->mmap_len, POSIX_MADV_SEQUENTIAL) < 0)
		die("posix_madvise");

	self->mmap_end	= mmap_window__end(self);

	return mmap_window__start(self);
}
