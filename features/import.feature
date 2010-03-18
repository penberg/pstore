Feature: CSV import
  In order to optimize read-performance of a CSV file
  As an user
  I want to import a CSV file into a pstore database

  Scenario: Uncompressed import
  Given a "32K" long CSV file
  When I execute "pstore import" command
  Then the database should contain the same data in column order

  Scenario: LZO compressed import
  Given a "32K" long CSV file
  When I execute "pstore import --compress=lzo" command
  Then the database should contain the same data in column order

  Scenario: FastLZ compressed import
  Given a "32K" long CSV file
  When I execute "pstore import --compress=fastlz" command
  Then the database should contain the same data in column order
