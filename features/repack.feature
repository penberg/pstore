Feature: pstore repack

  Scenario: Repack
    Given a 8K database named "test.pstore" based on a data file named "test.csv"
    When I run `pstore repack --compress=none --max-extent-len=1K test.pstore`
    Then the database named "test.pstore" should consist of the data file named "test.csv"

  Scenario: Repack after append
    Given a 16K data file named "additional.csv"
    And a 16K database named "test.pstore" based on a data file named "initial.csv"
    When I run `pstore import --append --max-extent-len=1K additional.csv test.pstore`
    And I run `pstore repack --compress=none --max-extent-len=1K test.pstore`
    Then the database named "test.pstore" should consist of the following data files:
      | initial.csv    |
      | additional.csv |
