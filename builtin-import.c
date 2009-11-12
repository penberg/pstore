#include "pstore/read-write.h"
#include "pstore/builtins.h"
#include "pstore/column.h"
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
	FILE		*input;
};

#define BUF_LEN	1024

static void csv_iterator_begin(void *private)
{
	struct csv_iterator_state *iter = private;
	char line[BUF_LEN];

	rewind(iter->input);

	/* Skip header row.  */
	if (fgets(line, BUF_LEN, iter->input) == NULL)
		die("fgets");
}

static void *csv_iterator_next(struct pstore_column *self, void *private)
{
	struct csv_iterator_state *iter = private;
	char line[BUF_LEN];
	char *s;

	if (fgets(line, BUF_LEN, iter->input) == NULL)
		return NULL;

	s = csv_field_value(line, self->column_id);
	if (!s)
		die("premature end of file");

	return s;
}

static void csv_iterator_end(void *private)
{
}

static struct pstore_iterator csv_iterator = {
	.begin		= csv_iterator_begin,
	.next		= csv_iterator_next,
	.end		= csv_iterator_end,
};

static void pstore_table__import_columns(struct pstore_table *self, FILE *input)
{
	char line[BUF_LEN];
	int field = 0;
	char *s;

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
	FILE *input;
	int output;

	if (argc != 4)
		usage();

	input = fopen(argv[2], "r");
	if (input == NULL)
		die("fopen: %s", strerror(errno));

	output = open(argv[3], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
	if (output < 0)
		die("open: %s", strerror(errno));

	header	= pstore_header__new();
	table	= pstore_table__new(argv[3], 0);

	pstore_header__insert_table(header, table);
	pstore_table__import_columns(table, input);

	pstore_header__write(header, output);

	state = (struct csv_iterator_state) {
		.input	= input,
	};
	pstore_table__import_values(table, output, &csv_iterator, &state);

	/*
	 * Write out the header again because offsets to data were not known
	 * until know.
	 */
	seek_or_die(output, 0, SEEK_SET);
	pstore_header__write(header, output);

	pstore_header__delete(header);

	fclose(input);
	close(output);

	return 0;
}
