#include "pstore/disk-format.h"
#include "pstore/mmap-window.h"
#include "pstore/read-write.h"
#include "pstore/builtins.h"
#include "pstore/builtins-common.h"
#include "pstore/column.h"
#include "pstore/compat.h"
#include "pstore/header.h"
#include "pstore/string.h"
#include "pstore/table.h"
#include "pstore/core.h"
#include "pstore/csv.h"
#include "pstore/die.h"
#include "pstore/row.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

struct csv_iterator_state {
	int			fd;
	off_t			file_size;

	struct mmap_window	*mmap;
	char			*pos;
};

#define MAX_WINDOW_LEN		MiB(128)
#define MAX_EXTENT_LEN		MiB(128)

static char			*input_file;
static char			*output_file;
static uint64_t			max_window_len = MAX_WINDOW_LEN;
struct pstore_import_details	details;

static char *csv_iterator_next_line(struct csv_iterator_state *iter)
{
	char *start = NULL;

restart:
	start = iter->pos;

	if (!mmap_window__in_window(iter->mmap, iter->pos))
		goto slide_mmap;

	while (*iter->pos != '\n') {
		iter->pos++;

		if (!mmap_window__in_window(iter->mmap, iter->pos))
			goto slide_mmap;
	}
	iter->pos++;
out:
	return start;

slide_mmap:
	if (!mmap_window__in_region(iter->mmap, iter->pos))
		return iter->pos = NULL;

	iter->pos = mmap_window__slide(iter->mmap, start);
	if (iter->pos == NULL)
		goto out;

	goto restart;
}

#define BUF_LEN	1024

static void csv_iterator_begin(void *private)
{
	struct csv_iterator_state *iter = private;

	iter->mmap = mmap_window__map(max_window_len, iter->fd, 0, iter->file_size);

	iter->pos = mmap_window__start(iter->mmap);

	/* Skip header row. */
	if (!csv_iterator_next_line(iter))
		die("premature end of file");
}

static bool csv_row_value(struct pstore_row *self, struct pstore_column *column, struct pstore_value *value)
{
	char *start = self->private;

	return csv_field_value(start, column->column_id, value);
}

static struct pstore_row_operations row_ops = {
	.row_value	= csv_row_value,
};

static bool csv_iterator_next(void *private, struct pstore_row *row)
{
	struct csv_iterator_state *iter = private;
	char *start;

	start = csv_iterator_next_line(iter);
	if (!start)
		return false;

	*row		= (struct pstore_row) {
		.private	= start,
		.ops		= &row_ops,
	};

	return true;
}

static void csv_iterator_end(void *private)
{
	struct csv_iterator_state *iter = private;

	mmap_window__unmap(iter->mmap);	
}

static struct pstore_iterator csv_iterator = {
	.begin		= csv_iterator_begin,
	.next		= csv_iterator_next,
	.end		= csv_iterator_end,
};

static void pstore_table__import_columns(struct pstore_table *self, const char *filename)
{
	char line[BUF_LEN];
	int field = 0;
	FILE *input;
	char *s;

	input = fopen(filename, "r");
	if (input == NULL)
		die("fopen: %s", strerror(errno));

	if (fgets(line, BUF_LEN, input) == NULL)
		die("fgets");

	for (;;) {
		struct pstore_column *column;
		struct pstore_value value;

		if (!csv_field_value(line, field, &value))
			break;

		s = strndup(value.s, value.len);
		if (!s)
			die("out of memory");

		column = pstore_column__new(s, field, VALUE_TYPE_STRING);

		pstore_table__add(self, column);

		free(s);
		field++;
	}
	fclose(input);
}

static int csv_nr_columns(const char *filename)
{
	int nr_columns;
	struct pstore_table *table;

	table = pstore_table__new(filename, 0);
	pstore_table__import_columns(table, filename);
	nr_columns = table->nr_columns;
	pstore_table__delete(table);

	return nr_columns;
}

static void usage(void)
{
	printf("\n usage: pstore import [OPTIONS] INPUT OUTPUT\n");
	printf("\n The options are:\n");
	printf("   -a, --append                 append data to existing database\n");
	printf("   -c, --compress SCHEME        set compression scheme (default: none)\n");
	printf("   -e, --max-extent-len LENGTH  set maximum extent length (default: 128M)\n");
	printf("   -w, --window-len LENGTH      set mmap window length (default: 128M)\n");
	comp_arg_usage();
	printf("\n");
	exit(EXIT_FAILURE);
}

static const struct option options[] = {
	{ "append",		no_argument,		NULL, 'a' },
	{ "compress",		required_argument,	NULL, 'c' },
	{ "max-extent-len",	required_argument,	NULL, 'e' },
	{ "window-len",		required_argument,	NULL, 'w' },
	{ }
};

static void parse_args(int argc, char *argv[])
{
	int ch;

	details.max_extent_len	= MAX_EXTENT_LEN;
	details.comp		= PSTORE_COMP_NONE;
	details.append		= false;

	while ((ch = getopt_long(argc, argv, "ac:e:w:", options, NULL)) != -1) {
		switch (ch) {
		case 'a':
			details.append		= true;
			break;
		case 'c':
			details.comp		= parse_comp_arg(optarg);
			break;
		case 'e':
			details.max_extent_len	= parse_storage_arg(optarg);
			break;
		case 'w':
			max_window_len		= parse_storage_arg(optarg);
			break;
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (details.comp >= NR_PSTORE_COMP)
		usage();

	input_file		= argv[0];
	output_file		= argv[1];
}

static int append(struct csv_iterator_state *state)
{
	struct pstore_header *header;
	struct pstore_table *table;
	int output;
	off_t f_offset;

	output = open(output_file, O_RDWR);
	if (output < 0)
		die("Failed to open output file '%s': %s", output_file, strerror(errno));

	f_offset = seek_or_die(output, 0, SEEK_END);

	if (posix_fallocate(output, f_offset, state->file_size) != 0)
		die("posix_fallocate");

	seek_or_die(output, 0, SEEK_SET);

	header = pstore_header__read(output);
	if (header->nr_tables != 1)
		die("number of tables does not match");

	table = header->tables[0];
	if (table->nr_columns != csv_nr_columns(input_file))
		die("number of columns does not match");

	pstore_table__import_values(table, output, &csv_iterator, state, &details);

	f_offset = seek_or_die(output, 0, SEEK_CUR);

	if (ftruncate(output, f_offset) != 0)
		die("ftruncate");

	/*
	 * Write out the header again because offsets to last extents changed.
	 */
	seek_or_die(output, 0, SEEK_SET);
	pstore_header__write(header, output);

	pstore_header__delete(header);

	if (fsync(output) < 0)
		die("fsync");

	if (close(output) < 0)
		die("close");

	return 0;
}

static int import(struct csv_iterator_state *state)
{
	struct pstore_header *header;
	struct pstore_table *table;
	int output;
	off_t f_size;

	output = open(output_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
	if (output < 0)
		die("Failed to open output file '%s': %s", output_file, strerror(errno));

	if (posix_fallocate(output, 0, state->file_size) != 0)
		die("posix_fallocate");

	header	= pstore_header__new();
	table	= pstore_table__new(output_file, 0);

	pstore_header__insert_table(header, table);
	pstore_table__import_columns(table, input_file);

	pstore_header__write(header, output);
	pstore_table__import_values(table, output, &csv_iterator, state, &details);

	f_size = seek_or_die(output, 0, SEEK_CUR);

	if (ftruncate(output, f_size) != 0)
		die("ftruncate");

	/*
	 * Write out the header again because offsets to data were not known
	 * until now.
	 */
	seek_or_die(output, 0, SEEK_SET);
	pstore_header__write(header, output);

	pstore_header__delete(header);

	if (fsync(output) < 0)
		die("fsync");

	if (close(output) < 0)
		die("close");

	return 0;
}

int cmd_import(int argc, char *argv[])
{
	struct csv_iterator_state state;
	int input;
	struct stat st;

	if (argc < 4)
		usage();

	parse_args(argc - 1, argv + 1);

	if (!input_file || !output_file)
		usage();

	input = open(input_file, O_RDONLY);
	if (input < 0)
		die("Failed to open input file '%s': %s", input_file, strerror(errno));

	if (fstat(input, &st) < 0)
		die("fstat");

	state = (struct csv_iterator_state) {
		.fd		= input,
		.file_size	= st.st_size,
	};

	if (details.append) {
		if (append(&state) < 0)
			die("append failed");
	}
	else {
		if (import(&state) < 0)
			die("import failed");
	}

	return 0;
}
