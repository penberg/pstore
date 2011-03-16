#include "pstore/builtins.h"
#include "pstore/segment.h"
#include "pstore/column.h"
#include "pstore/extent.h"
#include "pstore/header.h"
#include "pstore/table.h"
#include "pstore/die.h"

#include <sys/types.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

static char *input_file;

static void pstore_column__stat(struct pstore_column *self, int fd)
{
	struct pstore_segment *segment;
	struct pstore_extent *extent;

	printf("column:\n");
	printf("    name        : %s\n", self->name);
	printf("    column_id   : %" PRIu64 "\n", self->column_id);
	printf("    type        : %d\n", self->type);
	printf("    offset      : %" PRIu64 "\n", self->f_offset);
	printf("    last_extent : %" PRIu64 "\n", self->last_extent);

	segment = pstore_segment__read(self, fd);

	printf("    extents:\n");

	while ((extent = pstore_segment__next_extent(segment)) != NULL) {
		printf("       - lsize       : %" PRIu64 "\n", extent->lsize);
		printf("         psize       : %" PRIu64 "\n", extent->psize);
		printf("         comp        : %" PRIu8  "\n", extent->comp);
		printf("         next_extent : %" PRIu64 "\n", extent->next_extent);
		printf("\n");
	}

	pstore_segment__delete(segment);
}

static void pstore_table__stat(struct pstore_table *self, int fd)
{
	unsigned long ndx;

	printf("# table '%s'\n", self->name);

	for (ndx = 0; ndx < self->nr_columns; ndx++) {
		struct pstore_column *column = self->columns[ndx];

		pstore_column__stat(column, fd);
	}
}

static void pstore_header__stat(struct pstore_header *self, int fd)
{
	unsigned long ndx;

	for (ndx = 0; ndx < self->nr_tables; ndx++) {
		struct pstore_table *table = self->tables[ndx];

		pstore_table__stat(table, fd);
	}
}

static void parse_args(int argc, char *argv[])
{
	input_file		= argv[2];
}

static void usage(void)
{
	printf("\n usage: pstore stat INPUT\n\n");
	exit(EXIT_FAILURE);
}

int cmd_stat(int argc, char *argv[])
{
	struct pstore_header *header;
	struct stat st;
	int input;

	if (argc < 3)
		usage();

	parse_args(argc, argv);

	input = open(input_file, O_RDONLY);
	if (input < 0)
		die("open");

	if (fstat(input, &st) < 0)
		die("fstat");

	if (posix_fadvise(input, 0, st.st_size, POSIX_FADV_SEQUENTIAL) != 0)
		die("posix_fadvise");

	header = pstore_header__read(input);

	pstore_header__stat(header, input);

	pstore_header__delete(header);

	if (close(input) < 0)
		die("close");

	return 0;
}
