Feature: CSV export
  In order to easily access my data
  As a user
  I want to export a P-Store database into a CSV file

  Scenario: Export
  Given a 512K long CSV file
  When I run "pstore import --max-extent-len=1K"
  And I run "pstore export"
  Then the CSV files should be identical
