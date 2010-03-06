#include "pstore/compress.h"

#include "pstore/read-write.h"
#include "pstore/extent.h"
#include "pstore/buffer.h"
#include "pstore/die.h"

#include "minilzo/minilzo.h"

#include <stdlib.h>

#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

void pstore_extent__compress(struct pstore_extent *self, int fd)
{
	lzo_uint out_len;
	lzo_uint in_len;
	void *out;
	void *in;
	int err;

	in_len		= buffer__size(self->buffer);
	in		= buffer__start(self->buffer);

	out_len		= in_len + (in_len / 16 + 64 + 3);
	out		= malloc(out_len);
	if (!out)
		die("out of memory");

	err = lzo1x_1_compress(in, in_len, out, &out_len, wrkmem);
	if (err != LZO_E_OK)
		die("lzo1x_1_compress");

	if (out_len >= in_len)
		die("incompressible block");

	self->lsize	= in_len;

	write_or_die(fd, out, out_len);

	free(out);
}

