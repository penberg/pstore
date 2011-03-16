Feature: CSV import
  In order to optimize read-performance of a CSV file
  As an user
  I want to import a CSV file into a P-Store database

  Scenario: Uncompressed import
  Given a "32K" long CSV file
  When I run "pstore import" command
  Then the database should contain the same data in column order

  Scenario: FastLZ compressed import
  Given a "32K" long CSV file
  When I run "pstore import --compress=fastlz" command
  Then the database should contain the same data in column order

  Scenario: Uncompressed append
  Given a "2K" long CSV file
  And a "4K" long database
  When I run "pstore import --append" command
  Then the database should contain the same data in column order
