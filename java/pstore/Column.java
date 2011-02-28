package pstore;

public class Column {
  final long ptr;

  public Column(String name, long id, ValueType type) {
    ptr = create(name, id, type.value);
  }

  Column(long ptr) {
    this.ptr = ptr;
  }

  public void release() {
    destroy(ptr);
  }

  private static native long create(String name, long id, int type);
  private static native void destroy(long ptr);
}
