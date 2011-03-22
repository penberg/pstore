require 'rspec/expectations'
require 'fastercsv'
require 'tempfile'

Before do
  @csv_base_file      = Tempfile.open("csv-base")
  @csv_input_file     = Tempfile.open("csv-input")
  @pstore_output_file = Tempfile.open("pstore-output")
end

After do
  @csv_base_file.delete
  @csv_input_file.delete
  @pstore_output_file.delete
end

Given /^a ([^\ ]*) long CSV file$/ do |size|
  `./torture/gencsv #{@csv_input_file.path} #{size}`
end

Given /^a ([^\ ]*) long database$/ do |size|
  `./torture/gencsv #{@csv_base_file.path} #{size}`
  `./pstore import #{@csv_base_file.path} #{@pstore_output_file.path}`
end

When /^I run "pstore extend([^\"]*)"$/ do |options|
  `./pstore extend #{options} #{@pstore_output_file.path}`
end

When /^I run "pstore import([^\"]*)"$/ do |options|
  `./pstore import #{options} #{@csv_input_file.path} #{@pstore_output_file.path}`
end

When /^I run "pstore repack([^\"]*)"$/ do |options|
  `./pstore repack #{options} #{@pstore_output_file.path}`
end

Then /^the database should contain the same data in column order$/ do
  output = `./pstore cat #{@pstore_output_file.path} | grep -v "^#"`
  output.should == parse_csv(@csv_base_file.path, @csv_input_file.path)
end

def parse_csv(*files)
  values = Hash.new
  values.default = ""
  files.each do |file|
    FasterCSV.foreach(file, :headers => true) do |row|
      (0..row.length-1).each do |column|
        values[column] = values[column] + row[column] + "\n"
      end
    end
  end
  result = ""
  values.each_key do |key|
    result += values[key]
  end
  result
end
