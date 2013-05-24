#include "pstore/read-write.h"
#include "pstore/builtins.h"
#include "pstore/segment.h"
#include "pstore/compat.h"
#include "pstore/header.h"
#include "pstore/die.h"

#include <sys/types.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

static bool quiet_mode;
static char *input_file;

static void pstore_column_cat(struct pstore_column *self, int fd)
{
	struct pstore_segment *segment;
	char *s;

	printf("# Column: %s (ID = %" PRIu64 ", type = %d)\n", self->name, self->column_id, self->type);

	segment = pstore_segment_read(self, fd);

	while ((s = pstore_segment_next_value(segment)) != NULL) {
		if (!quiet_mode)
			 puts(s);
	}

	pstore_segment_delete(segment);
}

static void pstore_table_cat(struct pstore_table *self, int fd)
{
	unsigned long ndx;

	printf("# Table: %s\n", self->name);

	for (ndx = 0; ndx < self->nr_columns; ndx++) {
		struct pstore_column *column = self->columns[ndx];

		pstore_column_cat(column, fd);
	}
}

static void pstore_header_cat(struct pstore_header *self, int fd)
{
	unsigned long ndx;

	for (ndx = 0; ndx < self->nr_tables; ndx++) {
		struct pstore_table *table = self->tables[ndx];

		pstore_table_cat(table, fd);
	}
}

static void usage(void)
{
	printf("\n usage: pstore cat [OPTIONS] INPUT\n");
	printf("\n The options are:\n");
	printf("   -q, --quiet                  print only table and column headers\n");
	printf("\n");
	exit(EXIT_FAILURE);
}

static const struct option options[] = {
	{ "quiet",		no_argument,		NULL, 'q' },
	{ }
};

static void parse_args(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt_long(argc, argv, "q", options, NULL)) != -1) {
		switch (opt) {
		case 'q':
			quiet_mode = true;
			break;
		default:
			usage();
			break;
		}
	}

	argc -= optind;
	argv += optind;

	input_file		= argv[0];
}

int cmd_cat(int argc, char *argv[])
{
	struct pstore_header *header;
	struct stat st;
	int input;

	if (argc < 3)
		usage();

	parse_args(argc - 1, argv + 1);

	input = open(input_file, O_RDONLY);
	if (input < 0)
		die("open");

	if (fstat(input, &st) < 0)
		die("fstat");

	if (posix_fadvise(input, 0, st.st_size, POSIX_FADV_SEQUENTIAL) != 0)
		die("posix_fadvise");

	header = pstore_header_read(input);
	if (!header)
		die("pstore_header_read");

	pstore_header_cat(header, input);

	pstore_header_delete(header);

	if (close(input) < 0)
		die("close");

	return 0;
}
