Feature: CSV import
  In order to optimize read-performance of a CSV file
  As an user
  I want to import a CSV file into a pstore database

  Scenario: Import CSV as uncompressed database
  Given the following rows in a CSV:
    """
    Symbol,Close
    AAPL,223.02
    AAZN,128.82
    GOOG,564.34

    """
  When I execute the import command
  Then the database should contain the following data:
    """
    AAPL
    AAZN
    GOOG
    223.02
    128.82
    564.34

    """
