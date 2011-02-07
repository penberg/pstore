#include "pstore/compress.h"

#include "pstore/mmap-window.h"
#include "pstore/read-write.h"
#include "pstore/buffer.h"
#include "pstore/column.h"
#include "pstore/extent.h"
#include "pstore/core.h"
#include "pstore/die.h"

#include "quicklz/quicklz.h"
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
	struct mmap_window *mmap;
	void *out;
	void *in;
	int size;

	mmap		= mmap_window__map(self->psize, fd, offset + sizeof(struct pstore_file_extent), self->psize);
	in		= mmap_window__start(mmap);

	self->buffer	= buffer__new(self->lsize);
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

/*
 * 	QuickLZ
 */

static void extent__quicklz_compress(struct pstore_extent *self, int fd)
{
	void *scratch;
	int in_len;
	void *out;
	void *in;
	int size;

	in_len		= buffer__size(self->buffer);
	in		= buffer__start(self->buffer);

	out		= malloc(in_len + 400);
	if (!out)
		die("out of memory");

	scratch		= malloc(QLZ_SCRATCH_COMPRESS);
	if (!scratch)
		die("out of memory");

	/* XXX: errors? */
	size = qlz_compress(in, out, in_len, scratch);

	self->lsize	= in_len;

	write_or_die(fd, out, size);

	free(out);
	free(scratch);
}

static void *extent__quicklz_decompress(struct pstore_extent *self, int fd, off_t offset)
{
	struct mmap_window *mmap;
	void *scratch;
	void *out;
	void *in;
	int size;
	int len;

	mmap		= mmap_window__map(self->psize, fd, offset + sizeof(struct pstore_file_extent), self->psize);
	in		= mmap_window__start(mmap);

	len		= qlz_size_decompressed(in);
	if (len != self->lsize)
		die("error");

	self->buffer	= buffer__new(self->lsize);
	out		= buffer__start(self->buffer);

	scratch		= malloc(QLZ_SCRATCH_DECOMPRESS);
	if (!scratch)
		die("out of memory");

	size = qlz_decompress(in, out, scratch); 
	if (size != self->lsize)
		die("decompression failed");

	self->buffer->offset	= self->lsize;

	mmap_window__unmap(mmap);

	free(scratch);

	return out;
}

struct pstore_extent_ops extent_quicklz_ops = {
	.read		= extent__quicklz_decompress,
	.next_value	= extent__next_value,
	.flush		= extent__quicklz_compress,
};
