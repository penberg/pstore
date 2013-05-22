Feature: pstore import

  Scenario: Uncompressed import
    Given a 32K data file named "test.csv"
    When I run `pstore import test.csv test.pstore`
    Then the database named "test.pstore" should consist of the data file named "test.csv"

  Scenario: Uncompressed import from tab separated values
    Given a 32K data file named "test.tsv"
    When I run `pstore import --format=tsv test.tsv test.pstore`
    Then the database named "test.pstore" should consist of the data file named "test.tsv"

  Scenario: FastLZ compressed import
    Given a 32K data file named "test.csv"
    When I run `pstore import --compress=fastlz test.csv test.pstore`
    Then the database named "test.pstore" should consist of the data file named "test.csv"

  @snappy
  Scenario: Snappy compressed import
    Given a 32K data file named "test.csv"
    When I run `pstore import --compress=snappy test.csv test.pstore`
    Then the database named "test.pstore" should consist of the data file named "test.csv"

  Scenario: Uncompressed append
    Given a 2K data file named "additional.csv"
    And a 4K database named "test.pstore" based on a data file named "initial.csv"
    When I run `pstore import --append additional.csv test.pstore`
    Then the database named "test.pstore" should consist of the following data files:
      | initial.csv    |
      | additional.csv |

  Scenario: Append to existing table
    Given a 1K data file named "additional.csv"
    And a 1K database named "test.pstore" based on a data file named "initial.csv"
    When I run `pstore import --append --table=0 additional.csv test.pstore`
    Then the database named "test.pstore" should consist of the following data files:
      | initial.csv    |
      | additional.csv |

  Scenario: Append to non-existing table
    Given a 1K data file named "additional.csv"
    And a 1K database named "test.pstore" based on a data file named "initial.csv"
    When I run `pstore import --append --table=foo additional.csv test.pstore`
    Then it should fail with:
      """
      pstore_header_select_table: No such table: foo
      """

  Scenario: Significantly smaller window length than file size
    Given a 32K data file named "test.csv"
    When I run `pstore import --window-len=1K test.csv test.pstore`
    Then the database named "test.pstore" should consist of the data file named "test.csv"

  Scenario: Column detection for tab separated values
    Given a 32K data file named "test.tsv"
    When I run `pstore import --format=tsv test.tsv test.pstore`
    And I run `pstore stat test.pstore`
    Then the output should contain:
      """
      column_id    : 2
      """
