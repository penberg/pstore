#include "pstore/column.h"

#include "pstore/disk-format.h"
#include "pstore/read-write.h"
#include "pstore/buffer.h"
#include "pstore/extent.h"
#include "pstore/table.h"
#include "pstore/core.h"
#include "pstore/die.h"
#include "pstore/row.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

struct pstore_column *pstore_column__new(const char *name, uint64_t column_id, uint8_t type)
{
	struct pstore_column *self = calloc(sizeof *self, 1);

	if (!self)
		die("out of memory");

	self->name	= strdup(name);
	self->column_id	= column_id;
	self->type	= type;

	return self;
}

void pstore_column__delete(struct pstore_column *self)
{
	free(self->name);
	free(self);
}

struct pstore_column *pstore_column__read(int fd)
{
	struct pstore_file_column f_column;
	struct pstore_column *self;

	read_or_die(fd, &f_column, sizeof(f_column));

	self = pstore_column__new(f_column.name, f_column.column_id, f_column.type);

	self->f_offset = f_column.f_offset;

	return self;
}

void pstore_column__write(struct pstore_column *self, int fd)
{
	struct pstore_file_column f_column;

	f_column = (struct pstore_file_column) {
		.column_id	= self->column_id,
		.type		= self->type,
		.f_offset	= self->f_offset,
	};
	strncpy(f_column.name, self->name, PSTORE_COLUMN_NAME_LEN);

	write_or_die(fd, &f_column, sizeof(f_column));
}

void pstore_column__import_values(struct pstore_column *self,
				  int fd, struct pstore_iterator *iter,
				  void *private,
				  struct pstore_import_details *details)
{
	struct pstore_extent *extent;
	struct pstore_row row;

	extent = pstore_extent__new(self, details->comp);

	pstore_extent__prepare_write(extent, fd, details->max_extent_len);

	iter->begin(private);

	while (iter->next(self, private, &row)) {
		struct pstore_value value;

		if (!pstore_row__value(&row, self, &value))
			die("premature end of file");

		if (!pstore_extent__has_room(extent, &value)) {
			off_t offset;

			pstore_extent__flush_write(extent, fd);
			offset = seek_or_die(fd, 0, SEEK_CUR);
			pstore_extent__finish_write(extent, offset, fd);
			pstore_extent__prepare_write(extent, fd, details->max_extent_len);
		}
		pstore_extent__write_value(extent, &value, fd);
	}

	iter->end(private);

	pstore_extent__flush_write(extent, fd);
	pstore_extent__finish_write(extent, PSTORE_LAST_EXTENT, fd);
}
