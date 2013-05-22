#include "pstore/column.h"

#include "pstore/disk-format.h"
#include "pstore/read-write.h"
#include "pstore/buffer.h"
#include "pstore/extent.h"
#include "pstore/table.h"
#include "pstore/core.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

struct pstore_column *pstore_column_new(const char *name, uint64_t column_id, uint8_t type)
{
	struct pstore_column *self = calloc(sizeof *self, 1);

	if (!self)
		return NULL;

	self->name	= strdup(name);

	if (!self->name) {
		free(self);

		return NULL;
	}

	self->buffer	= buffer_new(0);

	self->column_id	= column_id;
	self->type	= type;

	return self;
}

void pstore_column_delete(struct pstore_column *self)
{
	buffer_delete(self->buffer);

	if (self->extent)
		pstore_extent_delete(self->extent);

	free(self->name);
	free(self);
}

struct pstore_column *pstore_column_read(int fd)
{
	struct pstore_file_column f_column;
	struct pstore_column *self;

	if (read_in_full(fd, &f_column, sizeof(f_column)) != sizeof(f_column))
		return NULL;

	self = pstore_column_new(f_column.name, f_column.column_id, f_column.type);

	self->first_extent	= f_column.first_extent;
	self->last_extent	= f_column.last_extent;

	return self;
}

int pstore_column_write(struct pstore_column *self, int fd)
{
	struct pstore_file_column f_column;

	f_column = (struct pstore_file_column) {
		.column_id	= self->column_id,
		.type		= self->type,
		.first_extent	= self->first_extent,
		.last_extent	= self->last_extent,
	};
	strncpy(f_column.name, self->name, PSTORE_COLUMN_NAME_LEN);

	if (write_in_full(fd, &f_column, sizeof(f_column)) != sizeof(f_column))
		return -1;

	return 0;
}

int pstore_column_flush_write(struct pstore_column *self, int fd)
{
	if (self->prev_extent != NULL) {
		off_t offset;

		offset = lseek(fd, 0, SEEK_CUR);
		if (offset < 0)
			return -1;

		if (pstore_extent_write_metadata(self->prev_extent, offset, fd) < 0)
			return -1;

		pstore_extent_delete(self->prev_extent);
	}
	pstore_extent_flush_write(self->extent, fd);

	return 0;
}

int pstore_column_preallocate(struct pstore_column *self, int fd, uint64_t extent_len)
{
	if (self->prev_extent != NULL) {
		off_t offset;

		offset = lseek(fd, 0, SEEK_CUR);
		if (offset < 0)
			return -1;

		if (pstore_extent_write_metadata(self->prev_extent, offset, fd) < 0)
			return -1;

		pstore_extent_delete(self->prev_extent);
	}

	return pstore_extent_preallocate(self->extent, fd, extent_len);
}
