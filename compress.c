#include "pstore/compress.h"

#include "pstore/mmap-window.h"
#include "pstore/read-write.h"
#include "pstore/buffer.h"
#include "pstore/column.h"
#include "pstore/extent.h"
#include "pstore/core.h"
#include "pstore/die.h"

#include "fastlz/fastlz.h"

#include <stdlib.h>
#include <stdio.h>

static void *extent__next_value(struct pstore_extent *self)
{
	char *start, *end;

	start = end = self->start;
	do {
		if (buffer__in_region(self->buffer, end))
			continue;

		return NULL;
	} while (*end++);
	self->start = end;

	return start;
}

/*
 * 	FastLZ
 */

static void extent__fastlz_compress(struct pstore_extent *self, int fd)
{
	int in_len;
	void *out;
	void *in;
	int size;

	in_len		= buffer__size(self->buffer);
	in		= buffer__start(self->buffer);

	out		= malloc(in_len * 2);	/* FIXME: buffer is too large */
	if (!out)
		die("out of memory");

	size = fastlz_compress(in, in_len, out);

	if (size >= in_len)
		fprintf(stdout, "warning: Column '%s' contains incompressible data.\n", self->parent->name);

	self->lsize	= in_len;

	write_or_die(fd, out, size);

	free(out);
}

static void *extent__fastlz_decompress(struct pstore_extent *self, int fd, off_t offset)
{
	struct pstore_column *column = self->parent;
	struct mmap_window *mmap;
	void *out;
	void *in;
	int size;

	column->work_mem = realloc(column->work_mem, self->lsize);
	if (!column->work_mem)
		die("out of memory");

	mmap		= mmap_window__map(self->psize, fd, offset + sizeof(struct pstore_file_extent), self->psize);
	in		= mmap_window__start(mmap);

	self->buffer	= buffer__new(column->work_mem, self->lsize);
	out		= buffer__start(self->buffer);

	size = fastlz_decompress(in, self->psize, out, self->lsize); 
	if (size != self->lsize)
		die("decompression failed");

	self->buffer->offset	= self->lsize;

	mmap_window__unmap(mmap);

	return out;
}

struct pstore_extent_ops extent_fastlz_ops = {
	.read		= extent__fastlz_decompress,
	.next_value	= extent__next_value,
	.flush		= extent__fastlz_compress,
};
