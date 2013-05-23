#include "pstore/header.h"

#include "pstore/disk-format.h"
#include "pstore/read-write.h"
#include "pstore/table.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct pstore_header *pstore_header_new(void)
{
	struct pstore_header *self = calloc(sizeof *self, 1);

	if (!self)
		return NULL;

	return self;
}

void pstore_header_delete(struct pstore_header *self)
{
	unsigned long ndx;

	for (ndx = 0; ndx < self->nr_tables; ndx++) {
		struct pstore_table *table = self->tables[ndx];

		pstore_table_delete(table);
	}

	free(self->tables);
	free(self);
}

int pstore_header_insert_table(struct pstore_header *self, struct pstore_table *table)
{
	void *p;

	self->nr_tables++;

	p = realloc(self->tables, sizeof(struct pstore_table *) * self->nr_tables);
	if (!p)
		return -ENOMEM;

	self->tables = p;

	self->tables[self->nr_tables - 1] = table;

	return 0;
}

struct pstore_header *pstore_header_read(int fd)
{
	struct pstore_file_table_idx f_index;
	struct pstore_file_header f_header;
	struct pstore_header *self;
	uint64_t nr;

	if (read_in_full(fd, &f_header, sizeof(f_header)) != sizeof(f_header))
		return NULL;

	if (memcmp(f_header.magic, PSTORE_MAGIC, PSTORE_MAGIC_LEN) != 0)
		return NULL;

	self = pstore_header_new();

	if (lseek(fd, f_header.t_index_offset, SEEK_SET) < 0)
		return NULL;

	if (read_in_full(fd, &f_index, sizeof(f_index)) != sizeof(f_index))
		return NULL;

	for (nr = 0; nr < f_index.nr_tables; nr++) {
		struct pstore_table *table = pstore_table_read(fd);

		pstore_header_insert_table(self, table);
	}

	return self;
}

int pstore_header_write(struct pstore_header *self, int fd)
{
	struct pstore_file_table_idx f_index;
	struct pstore_file_header f_header;
	off_t start_off, end_off;
	off_t t_index_off;
	unsigned long ndx;
	uint64_t size;

	t_index_off = lseek(fd, sizeof(f_header), SEEK_CUR);
	if (t_index_off < 0)
		return -1;

	start_off = lseek(fd, sizeof(f_index), SEEK_CUR);
	if (start_off < 0)
		return -1;

	for (ndx = 0; ndx < self->nr_tables; ndx++) {
		struct pstore_table *table = self->tables[ndx];

		if (pstore_table_write(table, fd) < 0)
			return -1;
	}
	end_off = lseek(fd, sizeof(f_header), SEEK_CUR);
	if (end_off < 0)
		return -1;

	size = end_off - start_off;

	if (lseek(fd, -(sizeof(f_header) + sizeof(f_index) + size), SEEK_CUR) < 0)
		return -1;

	f_header = (struct pstore_file_header) {
		.n_index_offset	= PSTORE_END_OF_CHAIN,
		.t_index_offset	= t_index_off,
	};
	strncpy(f_header.magic, PSTORE_MAGIC, PSTORE_MAGIC_LEN);

	if (write_in_full(fd, &f_header, sizeof(f_header)) != sizeof(f_header))
		return -1;

	f_index = (struct pstore_file_table_idx) {
		.nr_tables	= self->nr_tables,
		.t_index_next	= PSTORE_END_OF_CHAIN,
	};

	if (write_in_full(fd, &f_index, sizeof(f_index)) != sizeof(f_index))
		return -1;

	if (lseek(fd, sizeof(f_index) + size, SEEK_CUR) < 0)
		return -1;

	return 0;
}
