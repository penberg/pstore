require 'aruba/cucumber'

root_dir   = File.expand_path('../../..', __FILE__)
gencsv_dir = File.join(root_dir, 'tools/gencsv')

Before do
  set_env 'PATH', "#{root_dir}:#{gencsv_dir}:#{ENV['PATH']}"
end
