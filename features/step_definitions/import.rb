require 'rspec/expectations'
require 'fastercsv'
require 'tempfile'

Before do
  @pstore_file = Tempfile.open("pstore")
  @csv_file    = Tempfile.open("csv")
end

After do
  @pstore_file.delete
  @csv_file.delete
end

Given /^a "([^\"]*)" long CSV file$/ do |size|
  `./torture/gencsv #{@csv_file.path} #{size}`
end

When /^I execute "([^\"]*)" command$/ do |command|
  `./#{command} #{@csv_file.path} #{@pstore_file.path}`
end

Then /^the database should contain the same data in column order$/ do
  output = `./pstore cat #{@pstore_file.path} | grep -v "^#"`
  output.should == parse_csv(@csv_file.path)
end

def parse_csv(file)
  values = Hash.new
  values.default = ""
  FasterCSV.foreach(file, :headers => true) do |row|
    (0..row.length-1).each do |column|
      values[column] = values[column] + row[column] + "\n"
    end
  end
  result = ""
  values.each_key do |key|
    result += values[key]
  end
  result
end
