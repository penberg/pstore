#ifndef PSTORE_COMPRESS_H
#define PSTORE_COMPRESS_H

#include "pstore/extent.h"

extern struct pstore_extent_ops extent_fastlz_ops;

#ifdef CONFIG_HAVE_SNAPPY
extern struct pstore_extent_ops extent_snappy_ops;
#endif

#endif /* PSTORE_COMPRESS_H */
