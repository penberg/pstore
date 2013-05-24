Feature: pstore export

  Scenario: Export
    Given a 512K data file named "input.csv"
    When I run `pstore import --max-extent-len=1K input.csv test.pstore`
    And I run `pstore export test.pstore output.csv`
    Then the files named "input.csv" and "output.csv" should be equal
