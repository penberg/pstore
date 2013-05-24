Feature: pstore import

  Scenario: Uncompressed import
    Given a 32K long CSV file
    When I run "pstore import"
    Then the database should contain the same data in column order

  Scenario: Uncompressed import from tab separated values
    Given a 32K long TSV file
    When I run "pstore import --format=tsv"
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

  Scenario: Append to existing table
    Given a 1K long CSV file
    And a 1K long database
    When I run "pstore import --append --table=0"
    Then the database should contain the same data in column order

  Scenario: Append to non-existing table
    Given a 1K long CSV file
    And a 1K long database
    When I run "pstore import --append --table=foo"
    Then the error should be "pstore_header_select_table: No such table: foo"

  Scenario: Significantly smaller window length than file size
    Given a 32K long CSV file
    When I run "pstore import --window-len=1K"
    Then the database should contain the same data in column order

  Scenario: Column detection for tab separated values
    Given a 32K long TSV file
    When I run "pstore import --format=tsv"
    And I run "pstore stat"
    Then the output should contain:
      """
      column_id    : 2
      """
