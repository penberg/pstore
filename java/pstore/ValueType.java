package pstore;

import java.util.EnumSet;

public enum ValueType {
  STRING(0x01);

  final int value;

  private ValueType(int value) {
    this.value = value;
  }

  static ValueType parseInt(int value) {
    for (ValueType type : EnumSet.allOf(ValueType.class)) {
      if (type.value == value)
        return type;
    }
    throw new IllegalArgumentException("No such value: " + value);
  }
}
