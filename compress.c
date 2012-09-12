#include "pstore/compress.h"

#include "pstore/mmap-window.h"
#include "pstore/read-write.h"
#include "pstore/buffer.h"
#include "pstore/column.h"
#include "pstore/extent.h"
#include "pstore/bits.h"
#include "pstore/core.h"
#include "pstore/die.h"

#include "fastlz/fastlz.h"
#ifdef CONFIG_HAVE_SNAPPY
#include "snappy/snappy_compat.h"
#endif

#include <stdlib.h>
#include <stdio.h>

static void *extent_next_value(struct pstore_extent *self)
{
    void *start, *end;

    start = end = self->start;
    for (;;) {
        if (buffer_in_region(self->parent->buffer, end + sizeof(unsigned int))) {
            unsigned int *v = end;

            if (!has_zero_byte(*v)) {
                end += sizeof(unsigned int);
                continue;
            }
        }
        break;
    }

    while (buffer_in_region(self->parent->buffer, end)) {
        char *c = end++;
        if (!*c) {
            self->start = end;
            return start;
        }
    }
    return NULL;
}

/*
 *     FastLZ
 */

static void extent_fastlz_compress(struct pstore_extent *self, int fd)
{
    int in_len;
    void *out;
    void *in;
    int size;

    in_len        = buffer_size(self->write_buffer);
    in        = buffer_start(self->write_buffer);

    out        = malloc(in_len * 2);    /* FIXME: buffer is too large */
    if (!out)
        die("out of memory");

    size = fastlz_compress(in, in_len, out);

    if (size >= in_len)
        fprintf(stdout, "warning: Column '%s' contains incompressible data.\n", self->parent->name);

    self->lsize    = in_len;

    write_or_die(fd, out, size);

    free(out);
}

static void *extent_fastlz_decompress(struct pstore_extent *self, int fd, off_t offset)
{
    struct mmap_window *mmap;
    void *out;
    void *in;
    int size;

    buffer_resize(self->parent->buffer, self->lsize);

    mmap        = mmap_window_map(self->psize, fd, offset + sizeof(struct pstore_file_extent), self->psize);
    in        = mmap_window_start(mmap);

    out        = buffer_start(self->parent->buffer);

    size = fastlz_decompress(in, self->psize, out, self->lsize); 
    if (size != self->lsize)
        die("decompression failed");

    self->parent->buffer->offset    = self->lsize;

    mmap_window_unmap(mmap);

    return out;
}

struct pstore_extent_ops extent_fastlz_ops = {
    .read        = extent_fastlz_decompress,
    .next_value    = extent_next_value,
    .flush        = extent_fastlz_compress,
};

#ifdef CONFIG_HAVE_SNAPPY

/*
 *    Snappy
 */

static void extent_snappy_compress(struct pstore_extent *self, int fd)
{
    void    *input;
    size_t    input_length;
    void    *compressed;
    size_t    compressed_length;

    input_length    = buffer_size(self->write_buffer);
    input        = buffer_start(self->write_buffer);

    compressed    = malloc(snappy_max_compressed_length(input_length));
    if (!compressed)
        die("out of memory");

    snappy_raw_compress(input, input_length, compressed, &compressed_length);

    self->lsize    = input_length;

    write_or_die(fd, compressed, compressed_length);

    free(compressed);
}

static void *extent_snappy_decompress(struct pstore_extent *self, int fd, off_t offset)
{
    struct mmap_window *mmap;
    void *compressed;
    void *uncompressed;

    buffer_resize(self->parent->buffer, self->lsize);

    mmap        = mmap_window_map(self->psize, fd, offset + sizeof(struct pstore_file_extent), self->psize);
    compressed    = mmap_window_start(mmap);

    uncompressed    = buffer_start(self->parent->buffer);

    if (!snappy_raw_uncompress(compressed, self->psize, uncompressed))
        die("decompression failed");

    self->parent->buffer->offset    = self->lsize;

    mmap_window_unmap(mmap);

    return uncompressed;
}

struct pstore_extent_ops extent_snappy_ops = {
    .read        = extent_snappy_decompress,
    .next_value    = extent_next_value,
    .flush        = extent_snappy_compress,
};

#endif
