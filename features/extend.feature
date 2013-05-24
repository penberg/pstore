Feature: pstore extend

  Scenario: Extend
    Given a 8K data file named "test.csv"
    When I run `pstore import test.csv test.pstore`
    And I run `pstore extend --extent-len=8K test.pstore`
    Then the database named "test.pstore" should consist of the data file named "test.csv"

  Scenario: Append after extend
    Given a 8K data file named "additional.csv"
    And a 8K database named "test.pstore" based on a data file named "initial.csv"
    When I run `pstore extend --extent-len=16K test.pstore`
    And I run `pstore import --append additional.csv test.pstore`
    Then the database named "test.pstore" should consist of the following data files:
      | initial.csv    |
      | additional.csv |
