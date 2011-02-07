package pstore;

public enum ValueType {
  STRING(0x01);

  final int value;

  private ValueType(int value) {
    this.value = value;
  }
}
