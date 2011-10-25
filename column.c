#include "pstore/column.h"

#include "pstore/disk-format.h"
#include "pstore/read-write.h"
#include "pstore/buffer.h"
#include "pstore/extent.h"
#include "pstore/table.h"
#include "pstore/core.h"
#include "pstore/die.h"

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
		die("out of memory");

	self->name	= strdup(name);

	if (!self->name)
		die("out of memory");

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

	read_or_die(fd, &f_column, sizeof(f_column));

	self = pstore_column_new(f_column.name, f_column.column_id, f_column.type);

	self->first_extent	= f_column.first_extent;
	self->last_extent	= f_column.last_extent;

	return self;
}

void pstore_column_write(struct pstore_column *self, int fd)
{
	struct pstore_file_column f_column;

	f_column = (struct pstore_file_column) {
		.column_id	= self->column_id,
		.type		= self->type,
		.first_extent	= self->first_extent,
		.last_extent	= self->last_extent,
	};
	strncpy(f_column.name, self->name, PSTORE_COLUMN_NAME_LEN);

	write_or_die(fd, &f_column, sizeof(f_column));
}

void pstore_column_flush_write(struct pstore_column *self, int fd)
{
	if (self->prev_extent != NULL) {
		off_t offset;

		offset = seek_or_die(fd, 0, SEEK_CUR);

		pstore_extent_write_metadata(self->prev_extent, offset, fd);
		pstore_extent_delete(self->prev_extent);
	}
	pstore_extent_flush_write(self->extent, fd);
}

void pstore_column_preallocate(struct pstore_column *self, int fd, uint64_t extent_len)
{
	if (self->prev_extent != NULL) {
		off_t offset;

		offset = seek_or_die(fd, 0, SEEK_CUR);

		pstore_extent_write_metadata(self->prev_extent, offset, fd);
		pstore_extent_delete(self->prev_extent);
	}
	pstore_extent_preallocate(self->extent, fd, extent_len);
}
