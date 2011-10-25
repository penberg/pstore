#ifndef PSTORE_HEADER_H
#define PSTORE_HEADER_H

#include "pstore/table.h"

#include <stdint.h>

/* End of chain marker */
#define PSTORE_END_OF_CHAIN		(~0ULL)

struct pstore_header {
	unsigned long		nr_tables;
	struct pstore_table	**tables;
};

struct pstore_header *pstore_header_new(void);
void pstore_header_delete(struct pstore_header *self);
void pstore_header_insert_table(struct pstore_header *self, struct pstore_table *table);
struct pstore_header *pstore_header_read(int fd);
void pstore_header_write(struct pstore_header *self, int fd);

#endif /* PSTORE_HEADER_H */
