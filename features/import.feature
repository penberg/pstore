Feature: CSV import
  In order to optimize read-performance of a CSV file
  As a user
  I want to import a CSV file into a P-Store database

  Scenario: Uncompressed import
  Given a 32K long CSV file
  When I run "pstore import"
  Then the database should contain the same data in column order

  Scenario: FastLZ compressed import
  Given a 32K long CSV file
  When I run "pstore import --compress=fastlz"
  Then the database should contain the same data in column order

  @snappy
  Scenario: Snappy compressed import
  Given a 32K long CSV file
  When I run "pstore import --compress=snappy"
  Then the database should contain the same data in column order

  Scenario: Uncompressed append
  Given a 2K long CSV file
  And a 4K long database
  When I run "pstore import --append"
  Then the database should contain the same data in column order

  Scenario: Significantly smaller window length than file size
  Given a 32K long CSV file
  When I run "pstore import --window-len=1K"
  Then the database should contain the same data in column order
