#ifndef PSTORE_READ_WRITE_H
#define PSTORE_READ_WRITE_H

#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>

void read_or_die(int fd, void *buf, size_t count);
void write_or_die(int fd, const void *buf, size_t count);
off64_t seek_or_die(int fd, off64_t offset, int whence);

ssize_t xread(int fd, void *buf, size_t count);
ssize_t xwrite(int fd, const void *buf, size_t count);

ssize_t read_in_full(int fd, void *buf, size_t count);
ssize_t write_in_full(int fd, const void *buf, size_t count);

#endif /* PSTORE_READ_WRITE_H */
