#ifndef PSTORE_MMAP_WINDOW_H
#define PSTORE_MMAP_WINDOW_H

#include "pstore/compat.h"

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* A mmap sliding window */
struct mmap_window {
	size_t		mmap_pos;	/* start position in mmap window */
	size_t		mmap_len;	/* length of the mmap window */
	void		*mmap;		/* base pointer of the mmap window */
	void		*mmap_end;

	uint64_t	length;		/* length of the full region */
	uint64_t	pos;		/* position in the full region */

	off64_t		offset;		/* base offset of the region in fd */
	int		fd;
};

struct mmap_window *mmap_window__map(int fd, off64_t offset, off64_t length);
void mmap_window__unmap(struct mmap_window *self);
void *mmap_window__start(struct mmap_window *self);
void *mmap_window__slide(struct mmap_window *self, void *p);

static inline bool mmap_window__in_window(struct mmap_window *self, void *p)
{
	return (ssize_t)(p - self->mmap_end) < 0;
}

static inline bool mmap_window__in_region(struct mmap_window *self, void *p)
{
	size_t mmap_pos = p - self->mmap;

	return (int64_t)((uint64_t)(self->pos + mmap_pos) - self->length) < 0;
}

#endif /* PSTORE_MMAP_WINDOW_H */
