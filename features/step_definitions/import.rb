require 'spec/expectations'
require 'tempfile'

Before do
  @pstore_file = Tempfile.open("pstore")
  @csv_file    = Tempfile.open("csv")
end

After do
  @pstore_file.delete
  @csv_file.delete
end

Given /^a CSV file with the following content:$/ do |string|
  @csv_file.write string
  @csv_file.close
end

Given /^a CSV file$/ do
end

When /^I execute "([^\"]*)" command$/ do |command|
  `./#{command} #{@csv_file.path} #{@pstore_file.path}`
end

Then /^the database should contain the following data:$/ do |string|
  output = `./pstore cat #{@pstore_file.path} | grep -v "^#"`
  output.should == string
end
