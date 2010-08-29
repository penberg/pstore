#ifndef PSTORE_COMPRESS_H
#define PSTORE_COMPRESS_H

#include "pstore/extent.h"

extern struct pstore_extent_ops extent_lzo1x_1_ops;
extern struct pstore_extent_ops extent_fastlz_ops;
extern struct pstore_extent_ops extent_quicklz_ops;

#endif /* PSTORE_COMPRESS_H */
