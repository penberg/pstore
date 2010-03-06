#ifndef PSTORE_COMPRESS_H
#define PSTORE_COMPRESS_H

#include <sys/types.h>

struct pstore_extent;

void pstore_extent__compress(struct pstore_extent *self, int fd);
void pstore_extent__decompress(struct pstore_extent *self, int fd, off_t offset);

#endif /* PSTORE_COMPRESS_H */
