require 'aruba/cucumber'

root_dir   = File.expand_path('../../..', __FILE__)
gendsv_dir = File.join(root_dir, 'tools/gendsv')

Before do
  set_env 'PATH', "#{root_dir}:#{gendsv_dir}:#{ENV['PATH']}"
end
