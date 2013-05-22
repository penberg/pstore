#include "pstore/mmap-source.h"

#include "pstore/core.h"

#include <sys/mman.h>

struct mmap_source *mmap_source_alloc(int fd, off_t file_size, uint64_t max_window_len)
{
	struct mmap_source *self = calloc(1, sizeof(*self));

	if (!self)
		return NULL;

	if (max_window_len < PAGE_SIZE)
		max_window_len = PAGE_SIZE;

	self->fd		= fd;
	self->file_pos		= 0;
	self->file_size		= file_size;

	self->max_window_len	= max_window_len;

	self->mmap		= NULL;
	self->mmap_len		= 0;

	return self;
}

int mmap_source_read(void *source, const char **buffer, size_t *buffer_size)
{
	struct mmap_source *self = source;
	off_t file_rem;

	if (self->mmap != NULL) {
		if (munmap(self->mmap, self->mmap_len) != 0)
			return -1;

		self->mmap	= NULL;
		self->mmap_len	= 0;
	}

	self->mmap_len = self->max_window_len;

	file_rem = self->file_size - self->file_pos;
	if (file_rem < self->mmap_len)
		self->mmap_len = file_rem;

	if (self->mmap_len > 0) {
		self->mmap = mmap(NULL, self->mmap_len, PROT_READ, MAP_PRIVATE,
			self->fd, self->file_pos);
		if (self->mmap == MAP_FAILED)
			return -1;

		self->file_pos += self->mmap_len;
	}

	*buffer		= self->mmap;
	*buffer_size	= self->mmap_len;

	return 0;
}

void mmap_source_free(void *source)
{
	struct mmap_source *self = source;

	if (self->mmap)
		munmap(self->mmap, self->mmap_len);

	free(self);
}
