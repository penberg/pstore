/*
 * Copyright (c) 2012 Jussi Virtanen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SHEETS_H
#define SHEETS_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Sheets
 * ======
 *
 * Sheets is a simple and fast C library for reading tabular text formats,
 * such as CSV and TSV.
 *
 *     https://github.com/jvirtanen/sheets
 */

#define SHEETS_VERSION "0.3.3"

struct sheets_field
{
    const char *value;
    size_t      length;
};

struct sheets_reader;

struct sheets_record;

struct sheets_settings;

/*
 * Readers
 * -------
 */

/*
 * Allocate a reader that reads from the specified buffer.
 *
 * - buffer:      a buffer
 * - buffer_size: size of the buffer
 * - settings:    the settings for the reader
 *
 * If successful, returns a reader object. Otherwise returns `NULL`.
 */
struct sheets_reader *sheets_read_buffer(const char *, size_t,
    const struct sheets_settings *);

/*
 * Allocate a reader that reads from the specified file.
 *
 * - file:     a file
 * - settings: the settings for the reader
 *
 * If successful, returns a reader object. Otherwise returns `NULL`.
 */
struct sheets_reader *sheets_read_file(FILE *, const struct sheets_settings *);

/*
 * Deallocate the reader.
 *
 * - reader: the reader object
 */
void sheets_reader_free(struct sheets_reader *);

/*
 * Read a record. If successful, the operation updates the record object.
 * Otherwise the operation resets the record object. The operation fails at
 * end of input or upon error state.
 *
 * - reader: the reader object
 * - record: a record object
 *
 * If successful, returns zero. Otherwise returns non-zero.
 */
int sheets_reader_read(struct sheets_reader *, struct sheets_record *);

/*
 * Check whether the reader is in error state.
 *
 * - reader: the reader object
 *
 * Returns non-zero if the reader is in error state. Otherwise returns zero.
 */
int sheets_reader_error(const struct sheets_reader *);

enum sheets_error
{
    SHEETS_ERROR_TOO_BIG_RECORD = 1,
    SHEETS_ERROR_TOO_MANY_FIELDS = 2,
    SHEETS_ERROR_UNEXPECTED_CHARACTER = 3,
    SHEETS_ERROR_UNREADABLE_SOURCE = 4
};

/*
 * Records
 * -------
 */

/*
 * Allocate a record.
 *
 * - settings: the settings for the record
 *
 * If successful, returns a record object. Otherwise returns `NULL`.
 */
struct sheets_record *sheets_record_alloc(const struct sheets_settings *);

/*
 * Deallocate the record.
 *
 * - record: the record object
 */
void sheets_record_free(struct sheets_record *);

/*
 * Get the field at the specified index. If successful, the operation updates
 * the field object. Otherwise the operation does not alter the field object.
 * The operation fails if the index is too large.
 *
 * - record: the record object
 * - index:  an index
 * - field:  a field object
 *
 * If successful, returns zero. Otherwise returns non-zero.
 */
int sheets_record_field(const struct sheets_record *, unsigned int,
    struct sheets_field *);

/*
 * Get the number of fields in the record.
 *
 * - record: the record object
 *
 * Returns the number of fields in the record.
 */
size_t sheets_record_size(const struct sheets_record *);

/*
 * Default Settings
 * ----------------
 */

/*
 * Comma-separated values (CSV): a comma (`,`) is used as the delimiter and a
 * double quote (`"`) for quoting.
 */
extern const struct sheets_settings sheets_csv;

/*
 * Tab-separated values (TSV): a tab (`\t`) is used as the delimiter and
 * quoting is disabled.
 */
extern const struct sheets_settings sheets_tsv;

/*
 * Custom Settings
 * ---------------
 */

struct sheets_settings
{
    /*
     * The delimiter character. Must not be `\n` or `\r`.
     */
    char    delimiter;

    /*
     * The escape character. Must not be `\n`, `\r` or the delimiter character.
     * Set to `\0` to disable escaping. If escaping is enabled, quoting must be
     * disabled.
     */
    char    escape;

    /*
     * The quote character. Must not be `\n`, `\r` or the delimiter character.
     * Set to `\0` to disable quoting. If quoting is enabled, escaping must be
     * disabled.
     */
    char    quote;

    /*
     * Size of the buffer within a file source.
     */
    size_t  file_buffer_size;

    /*
     * Size of the buffer within a record.
     */
    size_t  record_buffer_size;

    /*
     * The maximum number of fields a record may contain.
     */
    size_t  record_max_fields;
};

#define SHEETS_MINIMUM_FILE_BUFFER_SIZE (1024)
#define SHEETS_DEFAULT_FILE_BUFFER_SIZE (4 * 1024)

#define SHEETS_MINIMUM_RECORD_BUFFER_SIZE (1024)
#define SHEETS_DEFAULT_RECORD_BUFFER_SIZE (1024 * 1024)

#define SHEETS_MINIMUM_RECORD_MAX_FIELDS (1)
#define SHEETS_DEFAULT_RECORD_MAX_FIELDS (1023)

/*
 * Custom Sources
 * --------------
 */

/*
 * Read from the source. If successful, the operation sets the pointer to a
 * buffer and the pointer to the size of the buffer. If the source is empty,
 * it sets the size of the buffer to zero.
 *
 * - source:      the source object
 * - buffer:      a pointer to a buffer
 * - buffer_size: a pointer to the size of the buffer
 *
 * If successful, returns zero. Otherwise returns non-zero.
 */
typedef int sheets_source_read_fn(void *, const char **, size_t *);

/*
 * Deallocate the source.
 *
 * - source: the source object
 */
typedef void sheets_source_free_fn(void *);

/*
 * Allocate a reader for the specified source.
 *
 * - source:   the source object
 * - read:     the read method
 * - free:     the free method
 * - settings: the settings for the reader
 *
 * If successful, returns a reader object. Otherwise returns `NULL`.
 */
struct sheets_reader *sheets_reader_alloc(void *, sheets_source_read_fn *,
    sheets_source_free_fn *, const struct sheets_settings *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SHEETS_H */
