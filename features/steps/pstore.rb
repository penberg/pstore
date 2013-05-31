require 'csv'

Given /^a ([^\ ]*) data file named "([^"]*)"$/ do |size, filename|
  generate_data_file(size, filename)
end

Given /^a ([^\ ]*) database named "([^"]*)" based on a data file named "([^"]*)"$/ do |size, database, filename|
  step "a #{size} data file named \"#{filename}\""
  step "I run `pstore import #{filename} #{database}`"
end

Then /^the files named "([^"]*)" and "([^"]*)" should be equal$/ do |a, b|
  step "I run `diff #{a} #{b}`"
  step "the output should contain exactly \"\""
end

Then /^the database named "([^"]*)" should consist of the data file named "([^"]*)"$/ do |database, filename|
  assert_database_content(database, [ filename ])
end

Then /^the database named "([^"]*)" should consist of the following data files:$/ do |database, filenames|
  assert_database_content(database, filenames.raw.map { |filename| filename.first })
end

def assert_database_content(database, filenames)
  run_simple("pstore cat #{database}")

  actual   = all_output.lines.select { |line| not line.start_with? '#' }.join
  expected = read_data_files(filenames.map { |filename| File.join(current_dir, filename) })

  assert_exact_output(expected, actual)
end

def generate_data_file(size, filename)
  if filename.end_with? '.tsv'
    run_simple("gendsv -s #{size} -t #{filename}")
  else
    run_simple("gendsv -s #{size} #{filename}")
  end
end

def read_data_files(filenames)
  columns = Hash.new("")

  filenames.each do |filename|
    CSV.foreach(filename, :headers => true, :col_sep => col_sep(filename)) do |row|
      for i in 0..row.length - 1
        columns[i] += "#{row[i]}\n"
      end
    end
  end

  columns.values.join
end

def col_sep(filename)
  if filename.end_with? '.tsv'
    "\t"
  else
    ","
  end
end
