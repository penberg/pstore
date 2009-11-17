#ifndef PSTORE_BLOCK_H
#define PSTORE_BLOCK_H

#include "pstore/mmap-window.h"
#include "pstore/column.h"

#include <stdint.h>

struct pstore_block {
	struct pstore_column	*parent;
	struct mmap_window	*mmap;
	char			*start;
};

struct pstore_block *pstore_block__new(void);
void pstore_block__delete(struct pstore_block *self);
struct pstore_block *pstore_block__read(struct pstore_column *column, int fd);
void *pstore_block__next_value(struct pstore_block *self);

#endif /* PSTORE_BLOCK_H */
