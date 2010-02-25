#include "pstore/column.h"

#include "pstore/disk-format.h"
#include "pstore/read-write.h"
#include "pstore/buffer.h"
#include "pstore/core.h"
#include "pstore/die.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define WRITEOUT_SIZE		KB(32)

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

static void pstore_column__write_value(struct pstore_column *self, struct buffer *buffer, void *p, size_t len)
{
	if (self->type != VALUE_TYPE_STRING)
		die("unknown type");

	buffer__append(buffer, p, len);
	buffer__append_char(buffer, '\0');
}

void pstore_column__import_values(struct pstore_column *self, int fd, struct pstore_iterator *iter, void *private)
{
	struct pstore_file_extent f_extent;
	uint64_t start_off, end_off;
	struct pstore_value value;
	struct buffer *buffer;
	uint64_t size;

	buffer = buffer__new(WRITEOUT_SIZE);

	iter->begin(private);

	start_off = seek_or_die(fd, sizeof(f_extent), SEEK_CUR);
	while (iter->next(self, private, &value)) {
		size_t len;

		len = value.len + 1;

		if (buffer__size(buffer) + len > WRITEOUT_SIZE) {
			write_or_die(fd, buffer__start(buffer), buffer__size(buffer));
			buffer__clear(buffer);
		}
		pstore_column__write_value(self, buffer, value.s, value.len);
	}
	if (buffer__size(buffer) > 0)
		write_or_die(fd, buffer__start(buffer), buffer__size(buffer));

	end_off = seek_or_die(fd, 0, SEEK_CUR);

	iter->end(private);

	size = end_off - start_off;

	seek_or_die(fd, -(sizeof(f_extent) + size), SEEK_CUR);
	f_extent = (struct pstore_file_extent) {
		.size	= size,
	};
	write_or_die(fd, &f_extent, sizeof(f_extent));

	seek_or_die(fd, size, SEEK_CUR);
}
