#include "pstore/disk-format.h"
#include "pstore/mmap-source.h"
#include "pstore/read-write.h"
#include "pstore/builtins.h"
#include "pstore/builtins-common.h"
#include "pstore/column.h"
#include "pstore/compat.h"
#include "pstore/header.h"
#include "pstore/string.h"
#include "pstore/table.h"
#include "pstore/core.h"
#include "pstore/die.h"
#include "pstore/row.h"
#include "sheets/sheets.h"

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

	struct sheets_reader	*reader;
	struct sheets_record	*record;
};

#define MAX_WINDOW_LEN		MiB(128)
#define MAX_EXTENT_LEN		MiB(128)

static char			*input_file;
static char			*output_file;
static char			*table_ref;
static uint64_t			max_window_len = MAX_WINDOW_LEN;
struct pstore_import_details	details;

#define BUF_LEN	1024

struct sheets_settings csv_iterator_settings = {
	.delimiter	= ',',
	.escape		= '\0',
	.quote		= '\0',
	.file_buffer_size	= SHEETS_DEFAULT_FILE_BUFFER_SIZE,
	.record_buffer_size	= SHEETS_DEFAULT_RECORD_BUFFER_SIZE,
	.record_max_fields	= SHEETS_DEFAULT_RECORD_MAX_FIELDS
};

static void csv_iterator_begin(void *private)
{
	struct csv_iterator_state *iter = private;

	struct mmap_source *source;

	source = mmap_source_alloc(iter->fd, iter->file_size, max_window_len);
	if (source == NULL)
		die("mmap_source_alloc");

	iter->reader = sheets_reader_alloc(source, &mmap_source_read, &mmap_source_free,
		&csv_iterator_settings);
	if (iter->reader == NULL)
		die("sheets_reader_alloc");

	iter->record = sheets_record_alloc(&csv_iterator_settings);
	if (iter->record == NULL)
		die("sheets_record_alloc");

	/* Skip header row. */
	if (sheets_reader_read(iter->reader, iter->record) != 0)
		die("premature end of file");
}

static bool csv_row_value(struct pstore_row *self, struct pstore_column *column, struct pstore_value *value)
{
	struct sheets_record *record = self->private;
	struct sheets_field field;

	if (sheets_record_field(record, column->column_id, &field) != 0)
		return false;

	value->s = field.value;
	value->len = field.length;

	return true;
}

static struct pstore_row_operations row_ops = {
	.row_value	= csv_row_value,
};

static bool csv_iterator_next(void *private, struct pstore_row *row)
{
	struct csv_iterator_state *iter = private;

	if (sheets_reader_read(iter->reader, iter->record) != 0)
		return false;

	*row		= (struct pstore_row) {
		.private	= iter->record,
		.ops		= &row_ops
	};

	return true;
}

static void csv_iterator_end(void *private)
{
	struct csv_iterator_state *iter = private;

	sheets_reader_free(iter->reader);
	sheets_record_free(iter->record);
}

static struct pstore_iterator csv_iterator = {
	.begin		= csv_iterator_begin,
	.next		= csv_iterator_next,
	.end		= csv_iterator_end,
};

static struct pstore_table *pstore_header_select_table(struct pstore_header *self)
{
	struct pstore_table *selected_table = NULL;
	unsigned long ndx;

	if (table_ref) {
		for (ndx = 0; ndx < self->nr_tables; ndx++) {
			struct pstore_table *table = self->tables[ndx];

			if (id_or_name_matches(table->table_id, table->name, table_ref))
				selected_table = table;
		}
		if (!selected_table)
			die("No such table: %s", table_ref);
	}
	else {
		if (self->nr_tables == 1)
			selected_table = self->tables[0];
		else
			die("Use '--table' to set table");
	}

	return selected_table;
}

static void pstore_table_import_columns(struct pstore_table *self, const char *filename)
{
	FILE *input;
	struct sheets_reader *reader;
	struct sheets_record *record;
	unsigned long ndx;

	input = fopen(filename, "r");
	if (input == NULL)
		die("fopen: %s", strerror(errno));

	reader = sheets_read_file(input, &csv_iterator_settings);
	if (reader == NULL)
		die("sheets_read_file");

	record = sheets_record_alloc(&csv_iterator_settings);
	if (record == NULL)
		die("sheets_record_alloc");

	if (sheets_reader_read(reader, record) != 0)
		die("sheets_reader_read");

	for (ndx = 0; ndx < sheets_record_size(record); ndx++) {
		struct sheets_field	field;
		struct pstore_column	*column;

		if (sheets_record_field(record, ndx, &field) != 0)
			die("sheets_record_field");

		column = pstore_column_new(field.value, ndx, VALUE_TYPE_STRING);
		if (column == NULL)
			die("pstore_column_new");

		if (pstore_table_add(self, column) < 0)
			die("pstore_table_add");
	}

	sheets_record_free(record);

	sheets_reader_free(reader);

	fclose(input);
}

static int csv_nr_columns(const char *filename)
{
	int nr_columns;
	struct pstore_table *table;

	table = pstore_table_new(filename, 0);
	pstore_table_import_columns(table, filename);
	nr_columns = table->nr_columns;
	pstore_table_delete(table);

	return nr_columns;
}

static void usage(void)
{
	printf("\n usage: pstore import [OPTIONS] INPUT OUTPUT\n");
	printf("\n The options are:\n");
	printf("   -a, --append                 append data to existing database\n");
	printf("   -c, --compress SCHEME        set compression scheme (default: none)\n");
	printf("   -d, --delimiter              set delimiter character (default: ',')\n");
	printf("   -e, --max-extent-len LENGTH  set maximum extent length (default: 128M)\n");
	printf("   -t, --table REF              set table (--append)\n");
	printf("   -u, --quote                  set quote character (default: none)\n");
	printf("   -w, --window-len LENGTH      set mmap window length (default: 128M)\n");
	comp_arg_usage();
	printf("\n");
	exit(EXIT_FAILURE);
}

static const struct option options[] = {
	{ "append",		no_argument,		NULL, 'a' },
	{ "compress",		required_argument,	NULL, 'c' },
	{ "delimiter",		required_argument,	NULL, 'd' },
	{ "max-extent-len",	required_argument,	NULL, 'e' },
	{ "table",		required_argument,	NULL, 't' },
	{ "quote",		required_argument,	NULL, 'u' },
	{ "window-len",		required_argument,	NULL, 'w' },
	{ }
};

static void parse_args(int argc, char *argv[])
{
	int ch;

	table_ref		= NULL;
	details.max_extent_len	= MAX_EXTENT_LEN;
	details.comp		= PSTORE_COMP_NONE;
	details.append		= false;

	while ((ch = getopt_long(argc, argv, "ac:d:e:t:u:w:", options, NULL)) != -1) {
		switch (ch) {
		case 'a':
			details.append			= true;
			break;
		case 'c':
			details.comp			= parse_comp_arg(optarg);
			break;
		case 'd':
			csv_iterator_settings.delimiter	= optarg[0];
			break;
		case 'e':
			details.max_extent_len		= parse_storage_arg(optarg);
			break;
		case 't':
			table_ref			= optarg;
			break;
		case 'u':
			csv_iterator_settings.quote	= optarg[0];
			break;
		case 'w':
			max_window_len			= parse_storage_arg(optarg);
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

	header = pstore_header_read(output);

	table = pstore_header_select_table(header);

	if (table->nr_columns != csv_nr_columns(input_file))
		die("number of columns does not match");

	pstore_table_import_values(table, output, &csv_iterator, state, &details);

	f_offset = seek_or_die(output, 0, SEEK_CUR);

	if (ftruncate(output, f_offset) != 0)
		die("ftruncate");

	/*
	 * Write out the header again because offsets to last extents changed.
	 */
	seek_or_die(output, 0, SEEK_SET);
	pstore_header_write(header, output);

	pstore_header_delete(header);

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

	header	= pstore_header_new();
	table	= pstore_table_new(output_file, 0);

	if (pstore_header_insert_table(header, table) < 0)
		die("pstore_header_insert_table");

	pstore_table_import_columns(table, input_file);

	if (pstore_header_write(header, output) < 0)
		die("pstore_header_write");

	pstore_table_import_values(table, output, &csv_iterator, state, &details);

	f_size = seek_or_die(output, 0, SEEK_CUR);

	if (ftruncate(output, f_size) != 0)
		die("ftruncate");

	/*
	 * Write out the header again because offsets to data were not known
	 * until now.
	 */
	seek_or_die(output, 0, SEEK_SET);

	if (pstore_header_write(header, output) < 0)
		die("pstore_header_write");

	pstore_header_delete(header);

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
