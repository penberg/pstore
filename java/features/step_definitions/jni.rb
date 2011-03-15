require 'rspec/expectations'

Before do
  @output = ""
end

After do
  @output = ""
end

When /^I run "([^"]*)" command$/ do |command|
  @output = `#{command}`
end

Then /^P-Store should output$/ do |text|
  @output.should == text
end
