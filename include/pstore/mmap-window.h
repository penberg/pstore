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
	void		*mmap;		/* start pointer of the mmap window */
	void		*mmap_end;	/* end pointer of the mmap window (exclusive) */

	uint64_t	offset;		/* offset in the full region */
	uint64_t	length;		/* length of the full region */

	uint64_t	start_off;	/* start offset in the file */
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
	uint64_t mmap_pos = p - self->mmap - self->mmap_pos;

	return (self->offset - self->start_off) + mmap_pos < self->length;
}

#endif /* PSTORE_MMAP_WINDOW_H */
