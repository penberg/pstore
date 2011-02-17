require 'rspec/expectations'

Before do
  @output = ""
end

After do
  @output = ""
end

Given /^a Java example program$/ do
end

When /^I run "([^"]*)" command$/ do |command|
  @output = `#{command}`
end

Then /^P-Store should output the example data$/ do
  @output.should == "col1.1\ncol2.1\ncol1.2\ncol2.2\ncol1.3\ncol2.3\n"
end
