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

  public String getName() {
    return name(ptr);
  }

  public long getId() {
    return id(ptr);
  }

  public ValueType getType() {
    return ValueType.parseInt(type(ptr));
  }

  private static native long create(String name, long id, int type);
  private static native void destroy(long ptr);
  private static native String name(long ptr);
  private static native long id(long ptr);
  private static native int type(long ptr);
}
