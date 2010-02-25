#include "pstore/mmap-window.h"
#include "pstore/read-write.h"
#include "pstore/builtins.h"
#include "pstore/column.h"
#include "pstore/compat.h"
#include "pstore/header.h"
#include "pstore/string.h"
#include "pstore/table.h"
#include "pstore/core.h"
#include "pstore/csv.h"
#include "pstore/die.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

struct csv_iterator_state {
	int			fd;
	off64_t			file_size;

	struct mmap_window	*mmap;
	char			*pos;
};

#define MMAP_WINDOW_LEN		MB(128)

static char		*input_file;
static char		*output_file;
static uint64_t		max_window_len = MMAP_WINDOW_LEN;

static char *csv_iterator_next_line(struct csv_iterator_state *iter)
{
	char *start = NULL;

restart:
	start = iter->pos;

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

static bool csv_iterator_next(struct pstore_column *self, void *private, struct pstore_value *value)
{
	struct csv_iterator_state *iter = private;
	char *start;

	start = csv_iterator_next_line(iter);
	if (!start)
		return false;

	if (!csv_field_value(start, self->column_id, value))
		die("premature end of file");

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

	input = fopen64(filename, "r");
	if (input == NULL)
		die("fopen64: %s", strerror(errno));

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

static unsigned long parse_int_arg(char *arg)
{
	char *start = strchr(arg, '=');

	return strtol(start + 1, NULL, 10);
}

static bool arg_matches(char *arg, const char *prefix)
{
	return strncmp(arg, prefix, strlen(prefix)) == 0;
}

static void parse_args(int argc, char *argv[])
{
	int ndx = 2;

	if (arg_matches(argv[ndx], "--window-len=")) {
		unsigned long x = parse_int_arg(argv[ndx++]);

		max_window_len = MB(x);
	}
	input_file		= argv[ndx++];
	output_file		= argv[ndx];
}

static void usage(void)
{
	printf("\n usage: pstore import INPUT OUTPUT\n\n");
	exit(EXIT_FAILURE);
}

int cmd_import(int argc, char *argv[])
{
	struct csv_iterator_state state;
	struct pstore_header *header;
	struct pstore_table *table;
	int input, output;
	struct stat64 st;

	if (argc < 4)
		usage();

	parse_args(argc, argv);

	input = open(input_file, O_RDONLY|O_LARGEFILE);
	if (input < 0)
		die("open: %s\n", strerror(errno));

	if (fstat64(input, &st) < 0)
		die("fstat");

	output = open(output_file, O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE, S_IRUSR|S_IWUSR);
	if (output < 0)
		die("open: %s", strerror(errno));

	if (posix_fallocate64(output, 0, st.st_size) != 0)
		die("posix_fallocate64");

	header	= pstore_header__new();
	table	= pstore_table__new(output_file, 0);

	pstore_header__insert_table(header, table);
	pstore_table__import_columns(table, input_file);

	pstore_header__write(header, output);

	state = (struct csv_iterator_state) {
		.fd		= input,
		.file_size	= st.st_size,
	};
	pstore_table__import_values(table, output, &csv_iterator, &state);

	/*
	 * Write out the header again because offsets to data were not known
	 * until know.
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
