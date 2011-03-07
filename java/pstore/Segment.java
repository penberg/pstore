package pstore;

public class Segment implements Releasable {
  final long ptr;

  public static Segment read(Column col, PStoreFile file) {
    return new Segment(read(col.ptr, file.fd));
  }

  Segment(long ptr) {
    this.ptr = ptr;
  }

  public void release() {
    destroy(ptr);
  }

  public String next() {
    return next(ptr);
  }

  private static native void destroy(long ptr);
  private static native long read(long ptr, int fd);
  private static native String next(long ptr);
}
