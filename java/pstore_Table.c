#include "pstore_Table.h"

#include "pstore-jni.h"
#include "pstore/core.h"
#include "pstore/disk-format.h"
#include "pstore/row.h"
#include "pstore/table.h"

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

static void iterator_nop(void *private)
{
}

static bool iterator_next(void *private, struct pstore_row *row)
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

static struct pstore_iterator iterator = {
	.begin = iterator_nop,
	.next = iterator_next,
	.end = iterator_nop,
};

JNIEXPORT void JNICALL Java_pstore_Table_importValues(JNIEnv *env, jclass clazz, jlong table_ptr, jint fd, jlong iter_state_ptr)
{
	struct pstore_import_details details = {
		.append		= false,
		.max_extent_len	= MAX_EXTENT_LEN,
		.comp		= PSTORE_COMP_NONE
	};

	struct iterator_state state = {
		.env = env,
		.obj = LONG_TO_PTR(iter_state_ptr)
	};

	pstore_table__import_values(LONG_TO_PTR(table_ptr), fd, &iterator,
				    &state, &details);
	lseek(fd, 0, SEEK_SET);
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
