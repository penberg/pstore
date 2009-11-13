#include "pstore/mmap-window.h"
#include "pstore/read-write.h"
#include "pstore/builtins.h"
#include "pstore/column.h"
#include "pstore/compat.h"
#include "pstore/header.h"
#include "pstore/string.h"
#include "pstore/table.h"
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

static char *csv_iterator_next_line(struct csv_iterator_state *iter)
{
	char *start;

restart:
	start = iter->pos;

	while (*iter->pos != '\n') {
		iter->pos++;

		if (!mmap_window__in_window(iter->mmap, iter->pos))
			goto slide_mmap;
	}
	iter->pos++;
out:
	return iter->pos;

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

	iter->mmap = mmap_window__map(iter->fd, 0, iter->file_size);

	iter->pos = mmap_window__start(iter->mmap);

	/* Skip header row. */
	if (!csv_iterator_next_line(iter))
		die("premature end of file");
}

static void *csv_iterator_next(struct pstore_column *self, void *private)
{
	struct csv_iterator_state *iter = private;
	char *s;

	if (!mmap_window__in_region(iter->mmap, iter->pos))
		return NULL;

	s = csv_field_value(iter->pos, self->column_id);
	if (!s)
		die("premature end of file");

	csv_iterator_next_line(iter);

	return s;
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

		s = csv_field_value(line, field);
		if (!s)
			break;

		column = pstore_column__new(s, field, VALUE_TYPE_STRING);

		pstore_table__add(self, column);

		free(s);
		field++;
	}
	fclose(input);
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

	if (argc != 4)
		usage();

	input = open(argv[2], O_RDONLY|O_LARGEFILE);
	if (input < 0)
		die("open: %s\n", strerror(errno));

	if (fstat64(input, &st) < 0)
		die("fstat");

	output = open(argv[3], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
	if (output < 0)
		die("open: %s", strerror(errno));

	header	= pstore_header__new();
	table	= pstore_table__new(argv[3], 0);

	pstore_header__insert_table(header, table);
	pstore_table__import_columns(table, argv[2]);

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

	close(output);

	return 0;
}
