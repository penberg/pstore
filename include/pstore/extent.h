#ifndef PSTORE_EXTENT_H
#define PSTORE_EXTENT_H

#include "pstore/disk-format.h"
#include "pstore/compat.h"

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

struct pstore_column;
struct pstore_value;
struct mmap_window;
struct buffer;

struct pstore_extent;

struct pstore_extent_ops {
    void *(*read)(struct pstore_extent *self, int fd, off_t offset);
    void *(*next_value)(struct pstore_extent *self);
    void (*flush)(struct pstore_extent *self, int fd);
    void (*prepare_write)(struct pstore_extent *self, int fd);
    void (*finish_write)(struct pstore_extent *self, int fd);
};

struct pstore_extent {
    const struct pstore_extent_ops    *ops;
    struct pstore_column        *parent;
    off_t                next_extent;
    uint64_t            lsize;
    uint64_t            psize;
    uint8_t                comp;

    /* read */
    struct mmap_window        *mmap;
    char                *start;

    /* write */
    struct buffer            *write_buffer;

    /* read and write */
    uint64_t            start_off;
    uint64_t            end_off;
};

struct pstore_extent *pstore_extent_new(struct pstore_column *parent, uint8_t comp);
void pstore_extent_delete(struct pstore_extent *self);
struct pstore_extent *pstore_extent_read(struct pstore_column *column, off_t offset, int fd);
void pstore_extent_prepare_write(struct pstore_extent *self, int fd, uint64_t max_extent_len);
void pstore_extent_prepare_append(struct pstore_extent *self);
void pstore_extent_flush_write(struct pstore_extent *self, int fd);
void pstore_extent_preallocate(struct pstore_extent *self, int fd, uint64_t extent_len);
void pstore_extent_write_metadata(struct pstore_extent *self, off_t next_extent, int fd);
void pstore_extent_write_value(struct pstore_extent *self, struct pstore_value *value, int fd);
bool pstore_extent_has_room(struct pstore_extent *self, struct pstore_value *value);

static inline bool pstore_extent_is_last(struct pstore_extent *self)
{
    return self->next_extent == PSTORE_LAST_EXTENT;
}

static inline void *pstore_extent_next_value(struct pstore_extent *self)
{
    return self->ops->next_value(self);
}

#endif /* PSTORE_EXTENT_H */
