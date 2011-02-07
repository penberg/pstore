#include "pstore_Row.h"

#include "pstore/column.h"
#include "pstore/die.h"
#include "pstore/row.h"
#include "pstore/value.h"
#include "pstore-jni.h"

#include <stdlib.h>
#include <string.h>

struct row_values {
  char **values;
  size_t nr_values;
};

static bool field_value(struct row_values *row_values, unsigned long field_ndx, struct pstore_value *value)
{
  if (field_ndx < row_values->nr_values) {
    value->s = row_values->values[field_ndx];
    value->len = strlen(value->s);
    return true;
  }
  return false;
}

static bool row_value(struct pstore_row *self, struct pstore_column *column, struct pstore_value *value)
{
  struct row_values *values = self->private;

  return field_value(values, column->column_id, value);
}

static struct pstore_row_operations row_ops = {
  .row_value  = row_value,
};

JNIEXPORT jlong JNICALL Java_pstore_Row_create(JNIEnv *env, jclass clazz, jobjectArray values)
{
  int i;
  struct pstore_row *row;
  struct row_values *row_values;

  row_values = calloc(sizeof(*row_values), 1);
  row_values->nr_values = (*env)->GetArrayLength(env, values);
  row_values->values = calloc(sizeof(char *), row_values->nr_values);

  for (i = 0; i < row_values->nr_values; i++) {
    const char *value0;
    jstring value;

    value = (jstring) (*env)->GetObjectArrayElement(env, values, i);
    if (!value)
      die("array element must not be NULL");

    value0 = (*env)->GetStringUTFChars(env, value, NULL);
    if (!value0)
      return PTR_TO_LONG(NULL);
    row_values->values[i] = strdup(value0);
    (*env)->ReleaseStringUTFChars(env, value, value0);
  }

  row = calloc(sizeof(struct pstore_row), 1);
  row->ops = &row_ops;
  row->private = row_values;

  return PTR_TO_LONG(row);
}

JNIEXPORT void JNICALL Java_pstore_Row_destroy(JNIEnv *env, jclass clazz, jlong ptr)
{
  size_t i;
  struct pstore_row *row;
  struct row_values *row_values;

  row = LONG_TO_PTR(ptr);
  row_values = row->private;

  for (i = 0; i < row_values->nr_values; i++)
    free(row_values->values[i]);

  free(row_values);
  free(row);
}
