#ifndef PSTORE_TABLE_H
#define PSTORE_TABLE_H

#include "pstore/column.h"

#include <stdbool.h>
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
	bool			append;
};

struct pstore_table *pstore_table_new(const char *name, uint64_t table_id);
void pstore_table_delete(struct pstore_table *self);
int pstore_table_add(struct pstore_table *self, struct pstore_column *column);
struct pstore_table *pstore_table_read(int fd);
int pstore_table_write(struct pstore_table *self, int fd);
int pstore_table_import_values(struct pstore_table *self, int fd, struct pstore_iterator *iter, void *private, struct pstore_import_details *details);
int pstore_table_export_values(struct pstore_table *self, struct pstore_iterator *iter, void *private, int output);

#endif /* PSTORE_TABLE_H */
