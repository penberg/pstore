#ifndef PSTORE_EXTENT_H
#define PSTORE_EXTENT_H

#include "pstore/mmap-window.h"
#include "pstore/column.h"

struct buffer;

struct pstore_extent {
	struct pstore_column	*parent;

	/* read */
	struct mmap_window	*mmap;
	char			*start;

	/* write */
	struct buffer		*buffer;
	uint64_t		start_off;
	uint64_t		end_off;
	uint64_t		size;
};

struct pstore_extent *pstore_extent__new(void);
void pstore_extent__delete(struct pstore_extent *self);
struct pstore_extent *pstore_extent__read(struct pstore_column *column, int fd);
void *pstore_extent__next_value(struct pstore_extent *self);
void pstore_extent__prepare_write(struct pstore_extent *self, int fd);
void pstore_extent__finish_write(struct pstore_extent *self, int fd);
void pstore_extent__write_value(struct pstore_extent *self, struct pstore_value *value, int fd);

#endif /* PSTORE_EXTENT_H */
