package pstore;

import java.util.ArrayList;
import java.util.List;

/**
 * The resources associated to an instance of <code>Header</code> must be
 * released using <code>Header#release</code> after which the inserted
 * <code>Tables</code> and associated <code>Columns</code> shall no longer be
 * accessed.
 */
public class Header implements Releasable {
  final long ptr;

  public static Header read(PStoreFile file) {
    return new Header(read(file.fd));
  }

  public Header() {
    ptr = create();
  }

  private Header(long ptr) {
    this.ptr = ptr;
  }

  public void release() {
    destroy(ptr);
  }

  public void insertTable(Table table) {
    insertTable(ptr, table.ptr);
  }

  public void write(PStoreFile file) {
    write(ptr, file.fd);
  }

  public List<Table> getTables() {
    List<Table> tables = new ArrayList<Table>();
    for (int ndx = 0; ndx < nrTables(ptr); ndx++)
      tables.add(new Table(tables(ptr, ndx)));
    return tables;
  }

  private static native long create();
  private static native void destroy(long ptr);
  private static native void write(long ptr, int fd);
  private static native void insertTable(long headerPtr, long tablePtr);
  private static native long read(int fd);
  private static native int nrTables(long ptr);
  private static native long tables(long ptr, int ndx);
}
