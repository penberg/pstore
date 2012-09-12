#include "pstore/segment.h"

#include "pstore/disk-format.h"
#include "pstore/column.h"
#include "pstore/extent.h"
#include "pstore/die.h"

#include <stddef.h>
#include <stdlib.h>

static struct pstore_segment *pstore_segment_new(void)
{
    struct pstore_segment *self = calloc(1, sizeof *self);

    if (!self)
        die("out of memory");

    return self;
}

void pstore_segment_delete(struct pstore_segment *self)
{
    if (self->map_extent)
        pstore_extent_delete(self->map_extent);
    free(self);
}

struct pstore_segment *pstore_segment_read(struct pstore_column *column, int fd)
{
    struct pstore_segment *self = pstore_segment_new();

    self->fd        = fd;
    self->parent        = column;
    self->map_extent    = pstore_extent_read(column, column->first_extent, self->fd);

    return self;
}

struct pstore_extent *pstore_segment_next_extent(struct pstore_segment *self)
{
    struct pstore_extent *ret = self->map_extent;

    if (!ret)
        return NULL;

    if (!pstore_extent_is_last(ret))
        self->map_extent    = pstore_extent_read(self->parent, self->map_extent->next_extent, self->fd);
    else
        self->map_extent    = NULL;

    return ret;
}

void *pstore_segment_next_value(struct pstore_segment *self)
{
    void *ret;

    ret = pstore_extent_next_value(self->map_extent);
    if (ret)
        return ret;

    if (pstore_extent_is_last(self->map_extent))
        return NULL;

    self->map_extent    = pstore_extent_read(self->parent, self->map_extent->next_extent, self->fd);

    return pstore_extent_next_value(self->map_extent);
}
