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

struct pstore_header *pstore_header__new(void);
void pstore_header__delete(struct pstore_header *self);
void pstore_header__insert_table(struct pstore_header *self, struct pstore_table *table);
struct pstore_header *pstore_header__read(int fd);
void pstore_header__write(struct pstore_header *self, int fd);

#endif /* PSTORE_HEADER_H */
