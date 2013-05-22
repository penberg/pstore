#include "pstore/read-write.h"
#include "pstore/builtins.h"
#include "pstore/builtins-common.h"
#include "pstore/segment.h"
#include "pstore/column.h"
#include "pstore/extent.h"
#include "pstore/header.h"
#include "pstore/table.h"
#include "pstore/core.h"
#include "pstore/die.h"

#include <sys/types.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#define EXTENT_LEN		MiB(16)

static char *input_file;
static char *table_ref;
static char *column_ref;
static uint64_t	extent_len;

static bool pstore_column_matches(struct pstore_column *self, char *ref)
{
	if (ref)
		return id_or_name_matches(self->column_id, self->name, ref);

	return true;
}

static bool pstore_table_matches(struct pstore_table *self, char *ref)
{
	if (ref)
		return id_or_name_matches(self->table_id, self->name, ref);

	return true;
}

static int extend_column(struct pstore_column *column, int input)
{
	struct pstore_extent *extent;
	off_t offset;

	offset = seek_or_die(input, 0, SEEK_CUR);

	extent = pstore_extent_read(column, column->last_extent, input);

	column->prev_extent = extent;
	column->extent = pstore_extent_new(column, PSTORE_COMP_NONE);

	seek_or_die(input, offset, SEEK_SET);

	if (pstore_column_preallocate(column, input, extent_len) < 0)
		return -1;

	if (pstore_extent_write_metadata(column->extent, PSTORE_LAST_EXTENT, input) < 0)
		return -1;

	return 0;
}

static int extend_table(struct pstore_table *table, int input)
{
	unsigned long ndx;

	for (ndx = 0; ndx < table->nr_columns; ndx++) {
		struct pstore_column *column = table->columns[ndx];

		if (pstore_column_matches(column, column_ref)) {
			if (extend_column(column, input) < 0)
				return -1;
		}
	}

	return 0;
}

static int extend(int input)
{
	struct pstore_header *header;
	unsigned long ndx;
	off_t f_end;

	header = pstore_header_read(input);

	seek_or_die(input, 0, SEEK_END);

	for (ndx = 0; ndx < header->nr_tables; ndx++) {
		struct pstore_table *table = header->tables[ndx];

		if (pstore_table_matches(table, table_ref)) {
			if (extend_table(table, input) < 0)
				return -1;
		}
	}

	f_end = seek_or_die(input, 0, SEEK_CUR);

	/*
	 * Write out the header again because offsets to last extents changed.
	 */
	seek_or_die(input, 0, SEEK_SET);
	if (pstore_header_write(header, input) < 0)
		die("pstore_header_write");

	pstore_header_delete(header);

	seek_or_die(input, f_end, SEEK_SET);

	return 0;
}

static const struct option options[] = {
	{ "column",		required_argument,	NULL, 'c' },
	{ "extent-len",		required_argument,	NULL, 'e' },
	{ "table",		required_argument,	NULL, 't' },
	{ }
};

static void usage(void)
{
	printf("\n usage: pstore extend [OPTIONS] INPUT\n");
	printf("\n The options are:\n");
	printf("   -c, --column REF             set column\n");
	printf("   -e, --extent-len LENGTH      set extent length (default: 16M)\n");
	printf("   -t, --table REF              set table\n");
	printf("\n");
	exit(EXIT_FAILURE);
}

static void parse_args(int argc, char *argv[])
{
	int ch;

	table_ref		= NULL;
	column_ref		= NULL;
	extent_len		= EXTENT_LEN;

	while((ch = getopt_long(argc, argv, "c:e:t:", options, NULL)) != -1) {
		switch (ch) {
		case 'c':
			column_ref	= optarg;
			break;
		case 'e':
			extent_len	= parse_storage_arg(optarg);
			break;
		case 't':
			table_ref	= optarg;
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

int cmd_extend(int argc, char *argv[])
{
	int input;
	off_t f_size;

	if (argc < 3)
		usage();

	parse_args(argc - 1, argv + 1);

	input = open(input_file, O_RDWR);
	if (input < 0)
		die("Failed to open input file '%s': %s", input_file, strerror(errno));

	f_size = seek_or_die(input, 0, SEEK_END);

	if (posix_fallocate(input, f_size, extent_len) != 0)
		die("posix_fallocate");

	seek_or_die(input, 0, SEEK_SET);

	if (extend(input) < 0)
		die("extend failed");

	f_size = seek_or_die(input, 0, SEEK_CUR);

	if (ftruncate(input, f_size) != 0)
		die("ftruncate");

	if (fsync(input) < 0)
		die("fsync");

	if (close(input) < 0)
		die("close");

	return 0;
}
