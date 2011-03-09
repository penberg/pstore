package pstore.examples;

import java.util.ArrayList;
import java.util.List;
import pstore.*;

import static pstore.PStoreFileMode.READ_ONLY;
import static pstore.PStoreFileMode.WRITE_ONLY;

public class PStore {
  static { System.loadLibrary("pstore-java"); }

  private static final String FILENAME = "/tmp/pstore-" + System.currentTimeMillis() + ".pstore";

  public static void main(String[] args) {
    writeDatabase();
    printDatabase();
  }

  private static void writeDatabase() {
    PStoreFile file = new PStoreFile(FILENAME, WRITE_ONLY);
    Table table = table();
    List<Row> rows = rows();
    writeTable(table, rows, file);
    for (Row row : rows) {
      row.release();
    }
    table.release();
    file.close();
  }

  private static void writeTable(Table table, List<Row> rows, PStoreFile output) {
    Header header = createHeader(table);
    header.write(output);
    IteratorState state = new IteratorState(rows.iterator());
    table.importValues(output, state);
    header.write(output);
    state.release();
    header.release();
  }

  private static void printDatabase() {
    PStoreFile input = new PStoreFile(FILENAME, READ_ONLY);
    Header header = Header.read(input);
    for (Table table : header.getTables())
      printTable(table, input);
  }

  private static void printTable(Table table, PStoreFile input) {
    printTableMetadata(table);
    for (Column col : table.getColumns())
      printColumn(col, input);
  }

  private static void printTableMetadata(Table table) {
    System.out.println("# Table: " + table.getName());
  }

  private static void printColumn(Column col, PStoreFile input) {
    printColumnMetadata(col);
    Segment segment = Segment.read(col, input);
    for (;;) {
      String value = segment.next();
      if (value == null)
        break;
      System.out.println(value);
    }
    segment.release();
  }

  private static void printColumnMetadata(Column col) {
    System.out.println("# Column: " + col.getName() + " (ID = " + col.getId() +
      ", type = " + col.getType().getValue() + ")");
  }

  private static Table table() {
    Table table = new Table("table", 0);
    for (Column column : columns())
      table.add(column);
    return table;
  }

  private static List<Column> columns() {
    List<Column> cols = new ArrayList<Column>();
    cols.add(new Column("Column1", 0, ValueType.STRING));
    cols.add(new Column("Column2", 1, ValueType.STRING));
    cols.add(new Column("Column3", 2, ValueType.STRING));
    return cols;
  }

  private static List<Row> rows() {
    List<Row> rows = new ArrayList<Row>();
    rows.add(new Row("col1.1", "col1.2", "col1.3"));
    rows.add(new Row("col2.1", "col2.2", "col2.3"));
    return rows;
  }

  private static Header createHeader(Table table) {
    Header header = new Header();
    header.insertTable(table);
    return header;
  }
}
