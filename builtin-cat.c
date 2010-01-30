#include "pstore/read-write.h"
#include "pstore/builtins.h"
#include "pstore/segment.h"
#include "pstore/header.h"
#include "pstore/die.h"

#include <sys/types.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

static bool quiet_mode;
static char *input_file;

static void pstore_column__cat(struct pstore_column *self, int fd)
{
	struct pstore_segment *segment;
	char *s;

	printf("# Column: %s (ID = %" PRIu64 ", type = %d)\n", self->name, self->column_id, self->type);

	segment = pstore_segment__read(self, fd);

	while ((s = pstore_segment__next_value(segment)) != NULL) {
		if (!quiet_mode)
			 printf("%s\n", s);
	}

	pstore_segment__delete(segment);
}

static void pstore_table__cat(struct pstore_table *self, int fd)
{
	unsigned long ndx;

	printf("# Table: %s\n", self->name);

	for (ndx = 0; ndx < self->nr_columns; ndx++) {
		struct pstore_column *column = self->columns[ndx];

		pstore_column__cat(column, fd);
	}
}

static void pstore_header__cat(struct pstore_header *self, int fd)
{
	unsigned long ndx;

	for (ndx = 0; ndx < self->nr_tables; ndx++) {
		struct pstore_table *table = self->tables[ndx];

		pstore_table__cat(table, fd);
	}
}

static bool arg_matches(char *arg, const char *prefix)
{
	return strncmp(arg, prefix, strlen(prefix)) == 0;
}

static void parse_args(int argc, char *argv[])
{
	int ndx = 2;

	if (arg_matches(argv[ndx], "-q")) {
		quiet_mode	= true;
		ndx++;
	}

	input_file		= argv[ndx];
}

static void usage(void)
{
	printf("\n usage: pstore cat INPUT\n\n");
	exit(EXIT_FAILURE);
}

int cmd_cat(int argc, char *argv[])
{
	struct pstore_header *header;
	int input;

	if (argc < 3)
		usage();

	parse_args(argc, argv);

	input = open(input_file, O_RDONLY|O_LARGEFILE);
	if (input < 0)
		die("open");

	header = pstore_header__read(input);

	pstore_header__cat(header, input);

	pstore_header__delete(header);

	if (close(input) < 0)
		die("close");

	return 0;
}
