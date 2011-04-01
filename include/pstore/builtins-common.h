#ifndef PSTORE_BUILTINS_COMMON_H
#define PSTORE_BUILTINS_COMMON_H

#include <stdbool.h>
#include <stdint.h>

unsigned long parse_storage_arg(char *arg);
uint8_t parse_comp_arg(char *arg);
void comp_arg_usage(void);
unsigned long parse_int_arg(char *arg);
bool is_int_arg(char *arg);
bool id_or_name_matches(uint64_t id, const char *name, char *ref);

#endif /* PSTORE_BUILTINS_COMMON_H */
