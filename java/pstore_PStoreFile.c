#include "pstore_PStoreFile.h"
#include "pstore/die.h"
#include "pstore-jni.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

enum PStoreFileMode {
	READ_ONLY		= 1,
	WRITE_ONLY		= 2
};

JNIEXPORT jint JNICALL Java_pstore_PStoreFile_open(JNIEnv * env, jclass clazz, jstring filename, jint mode)
{
	const char *output_file;
	int flags = 0;
	int fd;

	output_file = (*env)->GetStringUTFChars(env, filename, NULL);
	if (!output_file)
		return 0;

	if (mode == READ_ONLY)
		flags = O_RDONLY;
	else if (mode == WRITE_ONLY)
		flags = O_WRONLY | O_CREAT;
	else
		throw_io_exception(env, "Unknown file access mode: %d");

	fd = open(output_file, flags, S_IRUSR | S_IWUSR);
	(*env)->ReleaseStringUTFChars(env, filename, output_file);
	if (fd < 0)
		throw_io_exception(env, strerror(errno));

	return fd;
}

JNIEXPORT void JNICALL Java_pstore_PStoreFile_close(JNIEnv * env, jclass clazz, jint fd)
{
	if (close(fd) < 0)
		throw_io_exception(env, strerror(errno));
}
