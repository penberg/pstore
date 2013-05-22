Feature: pstore repack

  Scenario: Repack
  Given a 8K long CSV file
  When I run "pstore import"
  And I run "pstore repack --compress=none --max-extent-len=1K"
  Then the database should contain the same data in column order

  Scenario: Repack after append
  Given a 16K long CSV file
  And a 16K long database
  When I run "pstore import --append --max-extent-len=1K"
  And I run "pstore repack --compress=none --max-extent-len=1K"
  Then the database should contain the same data in column order
