package pstore;

import java.io.Closeable;

public class PStoreFile implements Closeable {
  final int fd;

  public PStoreFile(String filename, PStoreFileMode mode) {
    fd = open(filename, mode.value);
  }

  public void close() {
    close(fd);
  }

  private static native int open(String filename, int mode);
  private static native void close(int fd);
}
