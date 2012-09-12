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
#include <fcntl.h>
#include <stdio.h>

#define MAX_EXTENT_LEN        MiB(128)

static char *input_file;
struct pstore_import_details    details;

static int repack_extents(struct pstore_column *old_column, struct pstore_column *new_column, int input, int output)
{
    struct pstore_segment *segment;
    char *s;

    segment = pstore_segment_read(old_column, input);

    new_column->extent = pstore_extent_new(new_column, details.comp);
    pstore_extent_prepare_write(new_column->extent, output, details.max_extent_len);

    while ((s = pstore_segment_next_value(segment)) != NULL) {
        struct pstore_value value;

        value        = (struct pstore_value) {
            .s        = s,
            .len        = strlen(s),
        };

        if (!pstore_extent_has_room(new_column->extent, &value)) {
            pstore_column_flush_write(new_column, output);

            new_column->prev_extent = new_column->extent;

            new_column->extent = pstore_extent_new(new_column, details.comp);
            pstore_extent_prepare_write(new_column->extent, output, details.max_extent_len);
        }
        pstore_extent_write_value(new_column->extent, &value, output);
    }

    pstore_column_flush_write(new_column, output);
    pstore_extent_write_metadata(new_column->extent, PSTORE_LAST_EXTENT, output);

    pstore_segment_delete(segment);

    return 0;
}

static int repack_columns(struct pstore_table *old_table, struct pstore_table *new_table, int input, int output)
{
    unsigned long ndx;

    for (ndx = 0; ndx < new_table->nr_columns; ndx++) {
        struct pstore_column *new_column = new_table->columns[ndx];
        struct pstore_column *old_column = old_table->columns[ndx];

        if (repack_extents(old_column, new_column, input, output) < 0)
            return -1;
    }

    return 0;
}

static int repack_table(struct pstore_header *old_header, struct pstore_header *new_header, struct pstore_table *old_table, int output)
{
    struct pstore_table *new_table;
    unsigned long ndx;

    new_table    = pstore_table_new(old_table->name, old_table->table_id);

    pstore_header_insert_table(new_header, new_table);

    for (ndx = 0; ndx < old_table->nr_columns; ndx++) {
        struct pstore_column *old_column = old_table->columns[ndx];
        struct pstore_column *new_column;

        new_column = pstore_column_new(old_column->name, old_column->column_id, old_column->type);

        pstore_table_add(new_table, new_column);
    }

    return 0;
}

static int repack(int input, int output)
{
    struct pstore_header *new_header;
    struct pstore_header *old_header;
    unsigned long ndx;

    old_header    = pstore_header_read(input);

    new_header    = pstore_header_new();

    for (ndx = 0; ndx < old_header->nr_tables; ndx++) {
        struct pstore_table *table = old_header->tables[ndx];

        if (repack_table(old_header, new_header, table, output) < 0)
            return -1;
    }

    pstore_header_write(new_header, output);

    for (ndx = 0; ndx < new_header->nr_tables; ndx++) {
        struct pstore_table *new_table = new_header->tables[ndx];
        struct pstore_table *old_table = old_header->tables[ndx];

        if (repack_columns(old_table, new_table, input, output) < 0)
            return -1;
    }

    /*
     * Write out the header again because offsets to data were not known
     * until know.
     */
    seek_or_die(output, 0, SEEK_SET);
    pstore_header_write(new_header, output);

    pstore_header_delete(new_header);
    pstore_header_delete(old_header);

    return 0;
}

static void usage(void)
{
    printf("\n usage: pstore repack [OPTIONS] INPUT\n");
    printf("\n The options are:\n");
    printf("   -c, --compress SCHEME        set compression scheme (default: fastlz)\n");
    printf("   -e, --max-extent-len LENGTH  set maximum extent length (default: 128M)\n");
    comp_arg_usage();
    printf("\n");
    exit(EXIT_FAILURE);
}

static const struct option options[] = {
    { "compress",        required_argument,    NULL, 'c' },
    { "max-extent-len",    required_argument,    NULL, 'e' },
    { }
};

static void parse_args(int argc, char *argv[])
{
    int ch;

    details.max_extent_len    = MAX_EXTENT_LEN;
    details.comp        = PSTORE_COMP_FASTLZ;
    details.append        = false;

    while ((ch = getopt_long(argc, argv, "c:e:", options, NULL)) != -1) {
        switch (ch) {
        case 'c':
            details.comp        = parse_comp_arg(optarg);
            break;
        case 'e':
            details.max_extent_len    = parse_storage_arg(optarg);
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

    input_file        = argv[0];
}

int cmd_repack(int argc, char *argv[])
{
    char tmpname[PATH_MAX];
    struct stat st;
    int output;
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

    if (snprintf(tmpname, PATH_MAX, "%s.tmp", input_file) > PATH_MAX-1)
        die("snprintf");

    output = open(tmpname, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
    if (output < 0)
        die("open");

    if (repack(input, output) < 0)
        die("repack failed");

    if (close(input) < 0)
        die("close");

    if (fsync(output) < 0)
        die("fsync");

    if (close(output) < 0)
        die("close");

    if (rename(tmpname, input_file) < 0)
        die("rename");

    return 0;
}
