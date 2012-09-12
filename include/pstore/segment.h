#ifndef PSTORE_SEGMENT_H
#define PSTORE_SEGMENT_H

struct pstore_column;
struct pstore_extent;

struct pstore_segment {
    int                fd;
    struct pstore_column        *parent;
    struct pstore_extent        *map_extent;    /* currently mapped extent */
};

void pstore_segment_delete(struct pstore_segment *self);
struct pstore_segment *pstore_segment_read(struct pstore_column *column, int fd);
void *pstore_segment_next_value(struct pstore_segment *self);
struct pstore_extent *pstore_segment_next_extent(struct pstore_segment *self);

#endif /* PSTORE_SEGMENT_H */
