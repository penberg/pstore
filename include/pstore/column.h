#ifndef PSTORE_COLUMN_H
#define PSTORE_COLUMN_H

#include "pstore/value.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct pstore_import_details;
struct pstore_row;

struct pstore_column {
	char			*name;		/* column name */
	uint64_t		column_id;	/* unique ID */
	uint8_t			type;		/* type of data (see enum value_type) */
	uint64_t		f_offset;	/* offset of data in file */
};

struct pstore_column_iterator_state {
	char			*mmap;
	uint64_t		pos;
};

struct pstore_iterator {
	void (*begin)(void *private);
	bool (*next)(struct pstore_column *self, void *private, struct pstore_row *row);
	void (*end)(void *private);
};

struct pstore_column *pstore_column__new(const char *name, uint64_t column_id, uint8_t type);
void pstore_column__delete(struct pstore_column *self);
struct pstore_column *pstore_column__read(int fd);
void pstore_column__write(struct pstore_column *self, int fd);
void pstore_column__import_values(struct pstore_column *self, int fd, struct pstore_iterator *iter, void *private, struct pstore_import_details *details);

#endif /* PSTORE_COLUMN_H */
