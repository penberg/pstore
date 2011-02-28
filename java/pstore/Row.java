package pstore;

public class Row {
  final long ptr;

  public Row(String... values) {
    ptr = create(values);
  }

  public void release() {
    destroy(ptr);
  }

  private static native long create(String... values);
  private static native void destroy(long ptr);
}
