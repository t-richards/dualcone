# frozen_string_literal: true

# Disable warnings that don't belong to us
require 'warning'
Gem.path.each do |path|
  Warning.ignore(//, path)
end

require 'bundler/gem_tasks'

require 'rspec/core/rake_task'
RSpec::Core::RakeTask.new(:spec)

desc 'Run an encryption under valgrind memcheck tool'
task :memcheck do
  args = '--leak-check=yes --partial-loads-ok=yes --undef-value-errors=no'
  sh "valgrind #{args} ruby script/tester.rb"
end

desc 'Inspect built gem contents'
task inspect: :build do
  sh('cd pkg && tar xf *.gem && tar tvf data.tar.gz')
end

require 'rake/extensiontask'

task build: :compile

Rake::ExtensionTask.new('dualcone') do |ext|
  ext.lib_dir = 'lib/dualcone'
end

task default: %i[clobber compile spec]
