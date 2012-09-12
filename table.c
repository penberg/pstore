#include "pstore/table.h"

#include "pstore/disk-format.h"
#include "pstore/read-write.h"
#include "pstore/buffer.h"
#include "pstore/column.h"
#include "pstore/extent.h"
#include "pstore/header.h"
#include "pstore/value.h"
#include "pstore/core.h"
#include "pstore/die.h"
#include "pstore/row.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct pstore_table *pstore_table_new(const char *name, uint64_t table_id)
{
    struct pstore_table *self = calloc(sizeof *self, 1);

    if (!self)
        die("out of memory");

    self->name    = strdup(name);

    if (!self->name)
        die("out of memory");

    self->table_id    = table_id;

    return self;
}

void pstore_table_delete(struct pstore_table *self)
{
    unsigned long ndx;

    for (ndx = 0; ndx < self->nr_columns; ndx++) {
        struct pstore_column *column = self->columns[ndx];

        pstore_column_delete(column);
    }

    free(self->columns);
    free(self->name);
    free(self);
}

void pstore_table_add(struct pstore_table *self, struct pstore_column *column)
{
    void *p;

    self->nr_columns++;

    p = realloc(self->columns, sizeof(struct pstore_column *) * self->nr_columns);
    if (!p)
        die("out of memory");

    self->columns = p;

    self->columns[self->nr_columns - 1] = column;
}

struct pstore_table *pstore_table_read(int fd)
{
    struct pstore_file_table f_table;
    struct pstore_table *self;
    uint64_t nr;

    read_or_die(fd, &f_table, sizeof(f_table));

    self = pstore_table_new(f_table.name, f_table.table_id);

    for (nr = 0; nr < f_table.c_index.nr_columns; nr++) {
        struct pstore_column *column = pstore_column_read(fd);

        pstore_table_add(self, column);
    }

    return self;
}

void pstore_table_write(struct pstore_table *self, int fd)
{
    struct pstore_file_table f_table;
    uint64_t start_off, end_off;
    unsigned long ndx;
    uint64_t size;

    start_off = seek_or_die(fd, sizeof(f_table), SEEK_CUR);

    for (ndx = 0; ndx < self->nr_columns; ndx++) {
        struct pstore_column *column = self->columns[ndx];

        pstore_column_write(column, fd);
    }

    end_off = seek_or_die(fd, 0, SEEK_CUR);

    size = end_off - start_off;

    seek_or_die(fd, -(sizeof(f_table) + size), SEEK_CUR);

    f_table = (struct pstore_file_table) {
        .table_id    = self->table_id,

        .c_index    = (struct pstore_file_column_idx) {
            .nr_columns    = self->nr_columns,
            .c_index_next    = PSTORE_END_OF_CHAIN,
        },
    };
    strncpy(f_table.name, self->name, PSTORE_TABLE_NAME_LEN);

    write_or_die(fd, &f_table, sizeof(f_table));

    seek_or_die(fd, size, SEEK_CUR);
}

void pstore_table_import_values(struct pstore_table *self,
                 int fd, struct pstore_iterator *iter,
                 void *private,
                 struct pstore_import_details *details)
{
    struct pstore_row row;
    unsigned long ndx;

    /*
     * Prepare extents for appending
     */
    if (details->append) {
        for (ndx = 0; ndx < self->nr_columns; ndx++) {
            struct pstore_column *column = self->columns[ndx];
            struct pstore_extent *extent = pstore_extent_read(column, column->last_extent, fd);

            if (extent->comp == PSTORE_COMP_NONE) {
                column->extent = extent;
                pstore_extent_prepare_append(column->extent);
            }
            else
                column->prev_extent = extent;
        }

        seek_or_die(fd, 0, SEEK_END);
    }

    /*
     * Prepare extents
     */
    for (ndx = 0; ndx < self->nr_columns; ndx++) {
        struct pstore_column *column = self->columns[ndx];

        if (!column->extent) {
            column->extent = pstore_extent_new(column, details->comp);
            pstore_extent_prepare_write(column->extent, fd, details->max_extent_len);
        }
    }

    iter->begin(private);

    while (iter->next(private, &row)) {
        for (ndx = 0; ndx < self->nr_columns; ndx++) {
            struct pstore_column *column = self->columns[ndx];
            struct pstore_value value;

            if (!pstore_row_value(&row, column, &value))
                die("premature end of file");

            if (!pstore_extent_has_room(column->extent, &value)) {
                pstore_column_flush_write(column, fd);

                column->prev_extent = column->extent;

                column->extent = pstore_extent_new(column, details->comp);
                pstore_extent_prepare_write(column->extent, fd, details->max_extent_len);
            }
            pstore_extent_write_value(column->extent, &value, fd);
        }
    }

    iter->end(private);

    /*
     * Finish extents
     */
    for (ndx = 0; ndx < self->nr_columns; ndx++) {
        struct pstore_column *column = self->columns[ndx];

        pstore_column_flush_write(column, fd);

        pstore_extent_write_metadata(column->extent, PSTORE_LAST_EXTENT, fd);
    }
}

#define BUFFER_SIZE        MiB(128)

void pstore_table_export_values(struct pstore_table *self, struct pstore_iterator *iter, void *private, int output)
{
    struct pstore_row row;
    struct buffer *buffer;
    unsigned long ndx;

    buffer = buffer_new(BUFFER_SIZE);

    iter->begin(private);

    while (iter->next(private, &row)) {
        for (ndx = 0; ndx < self->nr_columns; ndx++) {
            struct pstore_column *column = self->columns[ndx];
            struct pstore_value value;

            if (!pstore_row_value(&row, column, &value))
                die("premature end of file");

            if (!buffer_has_room(buffer, value.len + 1)) {
                buffer_write(buffer, output);
                buffer_clear(buffer);
            }

            buffer_append(buffer, value.s, value.len);
            if (ndx == self->nr_columns - 1)
                buffer_append_char(buffer, '\n');
            else
                buffer_append_char(buffer, ',');
        }
    }

    iter->end(private);

    buffer_write(buffer, output);

    buffer_delete(buffer);
}
