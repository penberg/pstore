#ifndef SNAPPY_COMPAT_H
#define SNAPPY_COMPAT_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t snappy_max_compressed_length(size_t source_bytes);

void snappy_raw_compress(const char *input, size_t input_length, char *compressed, size_t *compressed_length);

int snappy_raw_uncompress(const char *compressed, size_t compressed_length, char *uncompressed);

#ifdef __cplusplus
}
#endif

#endif /* SNAPPY_COMPAT_H */
