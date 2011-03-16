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
	WRITE_ONLY		= 2,
	READ_WRITE		= 3
};

JNIEXPORT jint JNICALL Java_pstore_PStoreFile_open(JNIEnv * env, jclass clazz, jstring filename, jint mode)
{
	const char *file;
	int flags = 0;
	int fd;

	file = (*env)->GetStringUTFChars(env, filename, NULL);
	if (!file)
		return 0;

	switch (mode) {
	case READ_ONLY:
		flags = O_RDONLY;
		break;
	case WRITE_ONLY:
		flags = O_WRONLY | O_CREAT;
		break;
	case READ_WRITE:
		flags = O_RDWR;
		break;
	default:
		throw_io_exception(env, "Unknown file access mode: %d");
		break;
	}

	fd = open(file, flags, S_IRUSR | S_IWUSR);
	(*env)->ReleaseStringUTFChars(env, filename, file);
	if (fd < 0)
		throw_io_exception(env, strerror(errno));

	return fd;
}

JNIEXPORT void JNICALL Java_pstore_PStoreFile_close(JNIEnv * env, jclass clazz, jint fd)
{
	if (close(fd) < 0)
		throw_io_exception(env, strerror(errno));
}
