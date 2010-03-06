#ifndef PSTORE_TABLE_H
#define PSTORE_TABLE_H

#include "pstore/column.h"

#include <stdint.h>

struct pstore_table {
	char			*name;
	uint64_t		table_id;

	unsigned long		nr_columns;
	struct pstore_column	**columns;
};

struct pstore_import_details {
	uint64_t		max_extent_len;
	uint8_t			comp;
};

struct pstore_table *pstore_table__new(const char *name, uint64_t table_id);
void pstore_table__delete(struct pstore_table *self);
void pstore_table__add(struct pstore_table *self, struct pstore_column *column);
struct pstore_table *pstore_table__read(int fd);
void pstore_table__write(struct pstore_table *self, int fd);
void pstore_table__import_values(struct pstore_table *self, int fd, struct pstore_iterator *iter, void *private, struct pstore_import_details *details);

#endif /* PSTORE_TABLE_H */
