Feature: P-Store repack
  In order to save disk space
  As a user
  I want to optimize the on-disk layout of a P-Store database

  Scenario: Repack
  Given a 8K long CSV file
  When I run "pstore import"
  And I run "pstore repack --compress=none --max-extent-len=1K"
  Then the database should contain the same data in column order
