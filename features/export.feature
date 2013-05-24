Feature: pstore export

  Scenario: Export
    Given a 512K long CSV file
    When I run "pstore import --max-extent-len=1K"
    And I run "pstore export"
    Then the CSV files should be identical
