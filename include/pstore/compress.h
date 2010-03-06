#ifndef PSTORE_COMPRESS_H
#define PSTORE_COMPRESS_H

struct pstore_extent;

void pstore_extent__compress(struct pstore_extent *self, int fd);

#endif /* PSTORE_COMPRESS_H */
