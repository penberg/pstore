package pstore;

public enum PStoreFileMode {
  READ_ONLY(1), WRITE_ONLY(2), READ_WRITE(3);

  final int value;

  private PStoreFileMode(int value) {
    this.value = value;
  }
}
