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

Given /^the following rows in a CSV:$/ do |string|
  @csv_file.write string
  @csv_file.close
end

When /^I execute the import command$/ do
  `./pstore import #{@csv_file.path} #{@pstore_file.path}`
end

Then /^the database should contain the following data:$/ do |string|
  output = `./pstore cat #{@pstore_file.path} | grep -v "^#"`
  output.should == string
end
