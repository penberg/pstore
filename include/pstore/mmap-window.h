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

	uint64_t	max_window_len;	/* mmap window maximum length */

	uint64_t	pos;		/* position in the full region */
	uint64_t	length;		/* length of the full region */

	uint64_t	start_off;	/* start offset in the file */
	int		fd;
};

struct mmap_window *mmap_window__map(uint64_t window_len, int fd, off_t offset, off_t length);
void mmap_window__unmap(struct mmap_window *self);
void *mmap_window__start(struct mmap_window *self);
void *mmap_window__slide(struct mmap_window *self, void *p);

static inline uint64_t mmap_window__pos_in_window(struct mmap_window *self, void *p)
{
	return p - (self->mmap + self->mmap_pos);
}

static inline uint64_t mmap_window__pos_in_region(struct mmap_window *self, void *p)
{
	uint64_t win_pos = mmap_window__pos_in_window(self, p);

	return self->pos + self->mmap_pos + win_pos;
}

static inline bool mmap_window__in_window(struct mmap_window *self, void *p)
{
	return (ssize_t)(p - self->mmap_end) < 0;
}

static inline bool mmap_window__in_region(struct mmap_window *self, void *p)
{
	uint64_t pos = mmap_window__pos_in_region(self, p);

	return (int64_t)(pos - self->length) < 0;
}

#endif /* PSTORE_MMAP_WINDOW_H */
