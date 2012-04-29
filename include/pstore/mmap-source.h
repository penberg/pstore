#ifndef PSTORE_MMAP_SOURCE_H
#define PSTORE_MMAP_SOURCE_H

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

struct mmap_source {
	int		fd;
	off_t		file_pos;
	off_t		file_size;

	size_t		max_window_len;

	void		*mmap;
	size_t		mmap_len;
};

struct mmap_source *mmap_source_alloc(int fd, off_t file_size, uint64_t max_window_len);

int mmap_source_read(void *self, const char **buffer, size_t *buffer_size);

void mmap_source_free(void *self);

#endif /* PSTORE_MMAP_SOURCE_H */
