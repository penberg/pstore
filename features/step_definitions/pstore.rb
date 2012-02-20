require 'csv'
require 'rspec/expectations'
require 'tempfile'

Before do
  @csv_base_file      = Tempfile.open("csv-base")
  @csv_input_file     = Tempfile.open("csv-input")
  @csv_output_file    = Tempfile.open("csv-output")
  @pstore_output_file = Tempfile.open("pstore-output")
end

After do
  @csv_base_file.delete
  @csv_input_file.delete
  @csv_output_file.delete
  @pstore_output_file.delete
end

Given /^a ([^\ ]*) long CSV file$/ do |size|
  run "./tools/gencsv/gencsv #{@csv_input_file.path} #{size}"
end

Given /^a ([^\ ]*) long database$/ do |size|
  run "./tools/gencsv/gencsv #{@csv_base_file.path} #{size}"
  run "./pstore import #{@csv_base_file.path} #{@pstore_output_file.path}"
end

When /^I run "pstore export([^\"]*)"$/ do |options|
  run "./pstore export #{options} #{@pstore_output_file.path} #{@csv_output_file.path}"
end

When /^I run "pstore extend([^\"]*)"$/ do |options|
  run "./pstore extend #{options} #{@pstore_output_file.path}"
end

When /^I run "pstore import([^\"]*)"$/ do |options|
  run "./pstore import #{options} #{@csv_input_file.path} #{@pstore_output_file.path}"
end

When /^I run "pstore repack([^\"]*)"$/ do |options|
  run "./pstore repack #{options} #{@pstore_output_file.path}"
end

Then /^the database should contain the same data in column order$/ do
  run "./pstore cat #{@pstore_output_file.path} | grep -v \"^#\""
  @stdout.should == parse_csv(@csv_base_file.path, @csv_input_file.path)
end

Then /^the CSV files should be identical$/ do
  run "diff #{@csv_input_file.path} #{@csv_output_file.path}"
  @stdout.length.should == 0
end

Then /^the error should be "([^"]*)"$/ do |text|
  @stderr.chomp.should == text
end

def parse_csv(*files)
  values = Hash.new
  values.default = ""
  files.each do |file|
    CSV.foreach(file, :headers => true) do |row|
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
