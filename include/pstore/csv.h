#ifndef PSTORE_CSV_H
#define PSTORE_CSV_H

#include "pstore/column.h"

#include <stdbool.h>

bool csv_field_value(char *record, unsigned long field_idx, struct pstore_value *value);

#endif /* PSTORE_CSV_H */
