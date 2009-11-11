#include "pstore/read-write.h"
#include "pstore/column.h"
#include "pstore/block.h"
#include "pstore/die.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define COLUMN_NAME_LEN		32

struct pstore_file_column {
	char			name[COLUMN_NAME_LEN];
	uint64_t		column_id;
	uint64_t		type;
	uint64_t		f_offset;
};

struct pstore_file_block {
	uint64_t		size;
	uint64_t		block_next;
};

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
	strncpy(f_column.name, self->name, COLUMN_NAME_LEN);

	write_or_die(fd, &f_column, sizeof(f_column));
}

void pstore_column__write_value(struct pstore_column *self, int fd, void *p, size_t len)
{
	if (self->type != VALUE_TYPE_STRING)
		die("unknown type");

	write_or_die(fd, p, len);
}

void pstore_column__import_values(struct pstore_column *self, int fd, struct pstore_iterator *iter, void *private)
{
	struct pstore_file_block f_block;
	uint64_t start_off, end_off;
	uint64_t size;
	char *s;

	iter->begin(private);

	start_off = seek_or_die(fd, sizeof(f_block), SEEK_CUR);
	while ((s = iter->next(self, private)) != NULL) {
		pstore_column__write_value(self, fd, s, strlen(s) + 1);
		free(s);
	}
	end_off = seek_or_die(fd, 0, SEEK_CUR);

	iter->end(private);

	size = end_off - start_off;

	seek_or_die(fd, -(sizeof(f_block) + size), SEEK_CUR);
	f_block = (struct pstore_file_block) {
		.size	= size,
	};
	write_or_die(fd, &f_block, sizeof(f_block));

	seek_or_die(fd, size, SEEK_CUR);
}

struct pstore_block *pstore_block__read(struct pstore_column *column, int fd)
{
	struct pstore_file_block f_block;
	struct pstore_block *self;
	struct stat statbuf;
	void *p;

	self = pstore_block__new();

	seek_or_die(fd, column->f_offset, SEEK_SET);
	read_or_die(fd, &f_block, sizeof(f_block));

	if (fstat(fd, &statbuf) < 0)
		die("fstat");

	p = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (p == MAP_FAILED)
		die("mmap: %s", strerror(errno));

	self->parent	= column;
	self->mmap	= p;
	self->mmap_len	= statbuf.st_size;
	self->pos	= column->f_offset + sizeof(f_block);
	self->end	= self->pos + f_block.size;

	return self;
}
