#ifndef PSTORE_DISK_FORMAT_H
#define PSTORE_DISK_FORMAT_H

#include "pstore/types.h"

#include <stdint.h>

/*
 * This header file contains on-disk data structures of pstore files.
 */

#define PSTORE_MAGIC			"PSTORE02"
#define PSTORE_MAGIC_LEN		8

struct pstore_file_header {
	char			magic[PSTORE_MAGIC_LEN];
	le64			n_index_offset;
	le64			t_index_offset;
};

struct pstore_file_table_idx {
	le64			nr_tables;
	le64			t_index_next;
};

struct pstore_file_column_idx {
	le64			nr_columns;
	le64			c_index_next;
};

#define PSTORE_TABLE_NAME_LEN		32

struct pstore_file_table {
	char				name[PSTORE_TABLE_NAME_LEN];
	le64				table_id;
	struct pstore_file_column_idx	c_index;
};

#define PSTORE_COLUMN_NAME_LEN		32

struct pstore_file_column {
	char			name[PSTORE_COLUMN_NAME_LEN];
	le64			column_id;
	le64			type;
	le64			first_extent;
	le64			last_extent;
};

#define PSTORE_LAST_EXTENT		0ULL

enum pstore_comp {
	PSTORE_COMP_NONE	= 0,
	PSTORE_COMP_FASTLZ	= 2,
#ifdef CONFIG_HAVE_SNAPPY
	PSTORE_COMP_SNAPPY	= 3,
#endif

	NR_PSTORE_COMP		/* keep this last */
};

struct pstore_file_extent {
	le64			lsize;		/* logical size before compression */
	le64			psize;		/* physical size after compression */
	u8			comp;		/* compression algorithm */
	u8			padding[7];
	le64			next_extent;
};

#endif /* PSTORE_DISK_FORMAT_H */
