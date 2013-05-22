Feature: pstore extend

  Scenario: Extend
  Given a 8K long CSV file
  When I run "pstore import"
  And I run "pstore extend --extent-len=8K"
  Then the database should contain the same data in column order

  Scenario: Append after extend
  Given a 8K long CSV file
  And a 8K long database
  When I run "pstore extend --extent-len=16K"
  And I run "pstore import --append"
  Then the database should contain the same data in column order
