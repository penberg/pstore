#include "pstore_Table.h"

#include "pstore-jni.h"
#include "pstore/core.h"
#include "pstore/disk-format.h"
#include "pstore/row.h"
#include "pstore/table.h"
#include "pstore/segment.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_EXTENT_LEN MiB(128)

JNIEXPORT jlong JNICALL Java_pstore_Table_create(JNIEnv *env, jclass clazz, jstring name, jlong id)
{
	const char *name0;
	void *ptr;

	name0 = (*env)->GetStringUTFChars(env, name, NULL);
	if (!name0)
		return PTR_TO_LONG(NULL);
	ptr = pstore_table__new(name0, id);
	(*env)->ReleaseStringUTFChars(env, name, name0);

	return PTR_TO_LONG(ptr);
}

JNIEXPORT void JNICALL Java_pstore_Table_destroy(JNIEnv *env, jclass clazz, jlong ptr)
{
}

JNIEXPORT void JNICALL Java_pstore_Table_add(JNIEnv *env, jclass clazz, jlong table_ptr, jlong col_ptr)
{
	pstore_table__add(LONG_TO_PTR(table_ptr), LONG_TO_PTR(col_ptr));
}

struct iterator_state {
	JNIEnv *env;
	jobject obj;
};

struct export_iterator_state {
	int			fd;
	struct pstore_table	*table;

	struct pstore_segment	**segments;
	void			**row;
	JNIEnv			*env;
};

static void iterator_nop(void *private)
{
}

static bool import_iterator_next(void *private, struct pstore_row *row)
{
	struct iterator_state *state;
	jmethodID method_id;
	bool has_next;
	jclass clazz;
	jlong row_ptr;
	JNIEnv *env;

	state = private;
	env = state->env;

	clazz = (*env)->GetObjectClass(env, state->obj);
	method_id = (*env)->GetMethodID(env, clazz, "hasNext", "()Z");
	has_next = (*env)->CallBooleanMethod(env, state->obj, method_id);

	if (!has_next)
		return false;

	method_id = (*env)->GetMethodID(env, clazz, "next", "()J");
	row_ptr = (*env)->CallLongMethod(env, state->obj, method_id);
	memcpy(row, LONG_TO_PTR(row_ptr), sizeof(struct pstore_row));

	return true;
}

static struct pstore_iterator import_iterator = {
	.begin = iterator_nop,
	.next = import_iterator_next,
	.end = iterator_nop,
};

JNIEXPORT void JNICALL Java_pstore_Table_importValues(JNIEnv *env, jclass clazz, jlong table_ptr, jint fd, jlong iter_state_ptr, jboolean append)
{
	struct pstore_import_details details = {
		.append		= append,
		.max_extent_len	= MAX_EXTENT_LEN,
		.comp		= PSTORE_COMP_NONE
	};

	struct iterator_state state = {
		.env = env,
		.obj = LONG_TO_PTR(iter_state_ptr)
	};

	pstore_table__import_values(LONG_TO_PTR(table_ptr), fd, &import_iterator,
				    &state, &details);
	lseek(fd, 0, SEEK_SET);
}

static void export_iterator_begin(void *private)
{
	struct export_iterator_state *state = private;
	struct pstore_table *table = state->table;
	unsigned long ndx;

	state->segments = calloc(sizeof(struct pstore_segment *), table->nr_columns);
	if (!state->segments)
		goto throw_out_of_memory_error;

	for (ndx = 0; ndx < table->nr_columns; ndx++) {
		struct pstore_column *column = table->columns[ndx];
		state->segments[ndx] = pstore_segment__read(column, state->fd);
	}

	state->row = calloc(sizeof(void *), table->nr_columns);
	if (!state->row)
		goto error_free_segments;

	return;

error_free_segments:
	free(state->segments);

throw_out_of_memory_error:
	throw_out_of_memory_error(state->env, strerror(errno));
}

static bool pstore_table_row_value(struct pstore_row *self, struct pstore_column *column, struct pstore_value *value)
{
	struct export_iterator_state *state = self->private;
	struct pstore_table *table = state->table;
	char *str = NULL;
	unsigned long ndx;

	for (ndx = 0; ndx < table->nr_columns; ndx++) {
		if (table->columns[ndx]->column_id == column->column_id)
			str = state->row[ndx];
	}

	if (!str)
		return false;

	value->s	= str;
	value->len	= strlen(str);

	return true;
}

static struct pstore_row_operations row_ops = {
	.row_value	= pstore_table_row_value,
};

static bool export_iterator_next(void *private, struct pstore_row *row)
{
	struct export_iterator_state *state = private;
	struct pstore_table *table = state->table;
	unsigned long ndx;

	for (ndx = 0; ndx < table->nr_columns; ndx++)
		state->row[ndx] = pstore_segment__next_value(state->segments[ndx]);

	if (!state->row[0])
		return false;

	*row	= (struct pstore_row) {
		.private	= private,
		.ops		= &row_ops,
	};

	return true;
}

static void export_iterator_end(void *private)
{
	struct export_iterator_state *iter = private;
	struct pstore_table *table = iter->table;
	unsigned long ndx;

	for (ndx = 0; ndx < table->nr_columns; ndx++)
		pstore_segment__delete(iter->segments[ndx]);

	free(iter->segments);
	free(iter->row);
}

static struct pstore_iterator export_iterator = {
	.begin		= export_iterator_begin,
	.next		= export_iterator_next,
	.end		= export_iterator_end,
};

JNIEXPORT void JNICALL Java_pstore_Table_exportValues(JNIEnv *env, jclass clazz, jlong table_ptr, jint input, jint output)
{
	struct export_iterator_state state;

	state = (struct export_iterator_state) {
		.fd		= input,
		.table		= LONG_TO_PTR(table_ptr),
		.env		= env,
	};

	pstore_table__export_values(LONG_TO_PTR(table_ptr), &export_iterator, &state, output);
}

JNIEXPORT jint JNICALL Java_pstore_Table_nrColumns(JNIEnv *env, jclass clazz, jlong ptr)
{
	struct pstore_table *table = LONG_TO_PTR(ptr);

	return table->nr_columns;
}

JNIEXPORT jlong JNICALL Java_pstore_Table_columns(JNIEnv *env, jclass clazz, jlong ptr, jint ndx)
{
	struct pstore_table *table = LONG_TO_PTR(ptr);

	return PTR_TO_LONG(table->columns[ndx]);
}

JNIEXPORT jstring JNICALL Java_pstore_Table_name(JNIEnv *env, jclass clazz, jlong ptr)
{
  struct pstore_table *table = LONG_TO_PTR(ptr);
  return (*env)->NewStringUTF(env, table->name);
}

JNIEXPORT jlong JNICALL Java_pstore_Table_id(JNIEnv *env, jclass clazz, jlong ptr)
{
  struct pstore_table *table = LONG_TO_PTR(ptr);
  return table->table_id;
}
