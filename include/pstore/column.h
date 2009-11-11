#ifndef PSTORE_COLUMN_H
#define PSTORE_COLUMN_H

#include <stddef.h>
#include <stdint.h>

enum value_type {
	VALUE_TYPE_STRING		= 0x01,
};

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
	void *(*next)(struct pstore_column *self, void *private);
	void (*end)(void *private);
};

struct pstore_column *pstore_column__new(const char *name, uint64_t column_id, uint8_t type);
void pstore_column__delete(struct pstore_column *self);
struct pstore_column *pstore_column__read(int fd);
void pstore_column__write(struct pstore_column *self, int fd);
void pstore_column__write_value(struct pstore_column *self, int fd, void *p, size_t len);
void pstore_column__import_values(struct pstore_column *self, int fd, struct pstore_iterator *iter, void *private);

#endif /* PSTORE_COLUMN_H */
