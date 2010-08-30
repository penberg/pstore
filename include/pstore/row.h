#ifndef PSTORE_ROW_H
#define PSTORE_ROW_H

#include <stdbool.h>

struct pstore_column;
struct pstore_value;
struct pstore_row;

struct pstore_row_operations {
	bool (*row_value)(struct pstore_row *self, struct pstore_column *column, struct pstore_value *value);
};

struct pstore_row {
	void					*private;
	struct pstore_row_operations		*ops;
};

static inline bool pstore_row__value(struct pstore_row *self, struct pstore_column *column, struct pstore_value *value)
{
	return self->ops->row_value(self, column, value);
}

#endif /* PSTORE_ROW_H */
