#include "pstore/read-write.h"
#include "pstore/header.h"
#include "pstore/column.h"
#include "pstore/table.h"
#include "pstore/die.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct pstore_file_column_idx {
	uint64_t		nr_columns;
	uint64_t		c_index_next;
};

#define TABLE_NAME_LEN		32

struct pstore_file_table {
	char				name[TABLE_NAME_LEN];
	uint64_t			table_id;
	struct pstore_file_column_idx	c_index;
};

struct pstore_table *pstore_table__new(const char *name, uint64_t table_id)
{
	struct pstore_table *self = calloc(sizeof *self, 1);

	if (!self)
		die("out of memory");

	self->name	= strdup(name);
	self->table_id	= table_id;

	return self;
}

void pstore_table__delete(struct pstore_table *self)
{
	unsigned long ndx;

	for (ndx = 0; ndx < self->nr_columns; ndx++) {
		struct pstore_column *column = self->columns[ndx];

		pstore_column__delete(column);
	}

	free(self->columns);
	free(self->name);
	free(self);
}

void pstore_table__add(struct pstore_table *self, struct pstore_column *column)
{
	void *p;

	self->nr_columns++;

	p = realloc(self->columns, sizeof(struct pstore_column *) * self->nr_columns);
	if (!p)
		die("out of memory");

	self->columns = p;

	self->columns[self->nr_columns - 1] = column;
}

struct pstore_table *pstore_table__read(int fd)
{
	struct pstore_file_table f_table;
	struct pstore_table *self;
	uint64_t nr;

	read_or_die(fd, &f_table, sizeof(f_table));

	self = pstore_table__new(f_table.name, f_table.table_id);

	for (nr = 0; nr < f_table.c_index.nr_columns; nr++) {
		struct pstore_column *column = pstore_column__read(fd);

		pstore_table__add(self, column);
	}

	return self;
}

void pstore_table__write(struct pstore_table *self, int fd)
{
	struct pstore_file_table f_table;
	uint64_t start_off, end_off;
	unsigned long ndx;
	uint64_t size;

	start_off = seek_or_die(fd, sizeof(f_table), SEEK_CUR);

	for (ndx = 0; ndx < self->nr_columns; ndx++) {
		struct pstore_column *column = self->columns[ndx];

		pstore_column__write(column, fd);
	}

	end_off = seek_or_die(fd, 0, SEEK_CUR);

	size = end_off - start_off;

	seek_or_die(fd, -(sizeof(f_table) + size), SEEK_CUR);

	f_table = (struct pstore_file_table) {
		.table_id	= self->table_id,

		.c_index	= (struct pstore_file_column_idx) {
			.nr_columns	= self->nr_columns,
			.c_index_next	= PSTORE_END_OF_CHAIN,
		},
	};
	strncpy(f_table.name, self->name, TABLE_NAME_LEN);

	write_or_die(fd, &f_table, sizeof(f_table));

	seek_or_die(fd, size, SEEK_CUR);
}

void pstore_table__import_values(struct pstore_table *self, int fd, struct pstore_iterator *iter, void *private)
{
	unsigned long ndx;

	for (ndx = 0; ndx < self->nr_columns; ndx++) {
		struct pstore_column *column = self->columns[ndx];
		uint64_t f_offset;

		f_offset = seek_or_die(fd, 0, SEEK_CUR);

		pstore_column__import_values(column, fd, iter, private);

		column->f_offset = f_offset;
	}
}
