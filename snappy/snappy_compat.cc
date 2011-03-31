#include "snappy_compat.h"

#include <snappy.h>

size_t snappy_max_compressed_length(size_t source_bytes)
{
	return snappy::MaxCompressedLength(source_bytes);
}

void snappy_raw_compress(const char *input, size_t input_length, char *compressed, size_t *compressed_length)
{
        snappy::RawCompress(input, input_length, compressed, compressed_length);
}

int snappy_raw_uncompress(const char *compressed, size_t compressed_length, char *uncompressed)
{
        return snappy::RawUncompress(compressed, compressed_length, uncompressed);
}
