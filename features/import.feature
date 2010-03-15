Feature: CSV import
  In order to optimize read-performance of a CSV file
  As an user
  I want to import a CSV file into a pstore database

  Background:
    Given a CSV file with the following content:
    """
    Symbol,Close
    AAPL,223.02
    AAZN,128.82
    GOOG,564.34

    """

  Scenario: Uncompressed import
  Given a CSV file
  When I execute "pstore import" command
  Then the database should contain the following data:
    """
    AAPL
    AAZN
    GOOG
    223.02
    128.82
    564.34

    """

  Scenario: LZO compressed import
  Given a CSV file
  When I execute "pstore import --compress lzo" command
  Then the database should contain the following data:
    """
    AAPL
    AAZN
    GOOG
    223.02
    128.82
    564.34

    """

  Scenario: FastLZ compressed import
  Given a CSV file
  When I execute "pstore import --compress fastlz" command
  Then the database should contain the following data:
    """
    AAPL
    AAZN
    GOOG
    223.02
    128.82
    564.34

    """
