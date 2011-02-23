#include "pstore/header.h"

#include "pstore/disk-format.h"
#include "pstore/read-write.h"
#include "pstore/table.h"
#include "pstore/die.h"

#include <inttypes.h>
#include <stdlib.h>

static const char *pstore_magic = "PSTORE01";

#define PSTORE_MAGIC		(*(uint64_t *)pstore_magic)

struct pstore_header *pstore_header__new(void)
{
	struct pstore_header *self = calloc(sizeof *self, 1);

	if (!self)
		die("out of memory");

	return self;
}

void pstore_header__delete(struct pstore_header *self)
{
	unsigned long ndx;

	for (ndx = 0; ndx < self->nr_tables; ndx++) {
		struct pstore_table *table = self->tables[ndx];

		pstore_table__delete(table);
	}

	free(self->tables);
	free(self);
}

void pstore_header__insert_table(struct pstore_header *self, struct pstore_table *table)
{
	void *p;

	self->nr_tables++;

	p = realloc(self->tables, sizeof(struct pstore_table *) * self->nr_tables);
	if (!p)
		die("out of memory");

	self->tables = p;

	self->tables[self->nr_tables - 1] = table;
}

struct pstore_header *pstore_header__read(int fd)
{
	struct pstore_file_table_idx f_index;
	struct pstore_file_header f_header;
	struct pstore_header *self;
	uint64_t nr;

	read_or_die(fd, &f_header, sizeof(f_header));

	if (f_header.magic != PSTORE_MAGIC)
		die("bad magic: %" PRIX64, f_header.magic);

	self = pstore_header__new();

	seek_or_die(fd, f_header.t_index_offset, SEEK_SET);
	read_or_die(fd, &f_index, sizeof(f_index));

	for (nr = 0; nr < f_index.nr_tables; nr++) {
		struct pstore_table *table = pstore_table__read(fd);

		pstore_header__insert_table(self, table);
	}

	return self;
}

void pstore_header__write(struct pstore_header *self, int fd)
{
	struct pstore_file_table_idx f_index;
	struct pstore_file_header f_header;
	uint64_t start_off, end_off;
	uint64_t t_index_off;
	unsigned long ndx;
	uint64_t size;

	t_index_off = seek_or_die(fd, sizeof(f_header), SEEK_CUR);

	start_off = seek_or_die(fd, sizeof(f_index), SEEK_CUR);
	for (ndx = 0; ndx < self->nr_tables; ndx++) {
		struct pstore_table *table = self->tables[ndx];

		pstore_table__write(table, fd);
	}
	end_off = seek_or_die(fd, sizeof(f_header), SEEK_CUR);

	size = end_off - start_off;
	seek_or_die(fd, -(sizeof(f_header) + sizeof(f_index) + size), SEEK_CUR);

	f_header = (struct pstore_file_header) {
		.magic		= PSTORE_MAGIC,
		.n_index_offset	= PSTORE_END_OF_CHAIN,
		.t_index_offset	= t_index_off,
	};
	write_or_die(fd, &f_header, sizeof(f_header));

	f_index = (struct pstore_file_table_idx) {
		.nr_tables	= self->nr_tables,
		.t_index_next	= PSTORE_END_OF_CHAIN,
	};
	write_or_die(fd, &f_index, sizeof(f_index));

	seek_or_die(fd, sizeof(f_index) + size, SEEK_CUR);
}
