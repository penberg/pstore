#ifndef PSTORE_COLUMN_H
#define PSTORE_COLUMN_H

#include "pstore/value.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct pstore_import_details;
struct pstore_extent;
struct pstore_row;

struct pstore_column {
    char            *name;        /* column name */
    uint64_t        column_id;    /* unique ID */
    uint8_t            type;        /* type of data (see enum value_type) */
    uint64_t        first_extent;    /* offset to the first extent in file */
    uint64_t        last_extent;    /* offset to the last extent in file */

    struct pstore_extent    *prev_extent;
    struct pstore_extent    *extent;

    struct buffer        *buffer;
};

struct pstore_column_iterator_state {
    char            *mmap;
    uint64_t        pos;
};

struct pstore_iterator {
    void (*begin)(void *private);
    bool (*next)(void *private, struct pstore_row *row);
    void (*end)(void *private);
};

struct pstore_column *pstore_column_new(const char *name, uint64_t column_id, uint8_t type);
void pstore_column_delete(struct pstore_column *self);
struct pstore_column *pstore_column_read(int fd);
void pstore_column_write(struct pstore_column *self, int fd);
void pstore_column_flush_write(struct pstore_column *self, int fd);
void pstore_column_preallocate(struct pstore_column *self, int fd, uint64_t extent_len);

#endif /* PSTORE_COLUMN_H */
