#ifndef PSTORE_EXTENT_H
#define PSTORE_EXTENT_H

#include "pstore/mmap-window.h"
#include "pstore/column.h"

struct pstore_extent {
	struct pstore_column	*parent;
	struct mmap_window	*mmap;
	char			*start;
};

struct pstore_extent *pstore_extent__new(void);
void pstore_extent__delete(struct pstore_extent *self);
struct pstore_extent *pstore_extent__read(struct pstore_column *column, int fd);
void *pstore_extent__next_value(struct pstore_extent *self);

#endif /* PSTORE_EXTENT_H */
