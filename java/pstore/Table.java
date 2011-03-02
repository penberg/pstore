package pstore;

import java.util.ArrayList;
import java.util.List;

public class Table {
  final long ptr;

  public Table(String name, long id) {
    ptr = create(name, id);
  }

  Table(long ptr) {
    this.ptr = ptr;
  }

  // This Table instance takes the ownership of "col": Column#release() must not
  // be invoked after Table#add().
  public void add(Column col) {
    add(ptr, col.ptr);
  }

  public void importValues(PStoreFile file, IteratorState state) {
    importValues(ptr, file.fd, state.ptr);
  }

  public List<Column> getColumns() {
    List<Column> columns = new ArrayList<Column>();
    for (int ndx = 0; ndx < nrColumns(ptr); ndx++)
      columns.add(new Column(columns(ptr, ndx)));
    return columns;
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

  private static native long create(String name, long id);
  private static native void destroy(long ptr);
  private static native void add(long tablePtr, long colPtr);
  private static native void importValues(long tablePtr, int fd, long iterStatePtr);
  private static native int nrColumns(long ptr);
  private static native long columns(long ptr, int ndx);
  private static native String name(long ptr);
  private static native long id(long ptr);
}
