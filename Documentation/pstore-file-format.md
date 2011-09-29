P-Store File Format Specification
=================================

Version 0.1

## Notices

Copyright (c) 2009-2011 Pekka Enberg

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.3
or any later version published by the Free Software Foundation;
with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
You can find a copy of the license at [http://www.gnu.org/licenses/fdl-1.3.txt][fdl].

  [fdl]: http://www.gnu.org/licenses/fdl-1.3.txt

## Abstract

This document specifies the file format of P-Store, a simple read-optimized
database system.  P-Store stores data in column order which allows more
aggressive data compression and provides fast sequential access to subsets of
the stored columns.

## 1. File Structure

A P-Store file consists of a header, a table index, a column index (one for
each table), and a set of data segments (one for each column) which hold the
actual values of a column. A segment is a set of extents allocated for a single
column and an extent is a contiguous area of storage allocated from the file
system.  Whether an extent is physically contiguous on disk or not is up to the
backing filesystem.

### 1.1 Header

The P-Store file header has the following format:

    struct pstore_file_header {
            uint64_t                magic;
            uint64_t                n_index_offset;
            uint64_t                t_index_offset;
    };

Magic: `0x53 0x50 0x4f 0x54 0x45 0x52 0x30 0x32`

### 1.2 Table Index

The table index has the following format:

    struct pstore_file_table_idx {
            uint64_t                nr_tables;
            uint64_t                t_index_next;
    };

### 1.3 Table

A table has the following format:

    #define TABLE_NAME_LEN          32

    struct pstore_file_table {
            char                            name[TABLE_NAME_LEN];
            uint64_t                        table_id;
            struct pstore_file_column_idx   c_index;
    };

### 1.4 Column Index

A column index has the following format:

    struct pstore_file_column_idx {
            uint64_t                nr_columns;
            uint64_t                c_index_next;
    };

### 1.5 Column

A column has the following format:

    #define COLUMN_NAME_LEN         32

    struct pstore_file_column {
            char                    name[COLUMN_NAME_LEN];
            uint64_t                column_id;
            uint64_t                type;
            uint64_t                first_extent;
            uint64_t                last_extent;
    };

### 1.6 Extent

An extent has the following format:

    struct pstore_file_extent {
            uint64_t                lsize;
            uint64_t                psize;
            uint8_t                 comp;
            uint8_t                 padding[7];
            uint64_t                next_extent;
    };
