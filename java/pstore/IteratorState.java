package pstore;

import java.util.Iterator;

public class IteratorState implements Releasable {
  private Iterator<Row> rowIterator;
  final long ptr;

  public IteratorState(Iterator<Row> rowIterator) {
    this.rowIterator = rowIterator;
    ptr = create();
  }

  public void release() {
    destroy(ptr);
  }

  private boolean hasNext() {
    return rowIterator.hasNext();
  }

  private long next() {
    return rowIterator.next().ptr;
  }

  private native long create();
  private native void destroy(long ptr);
}
