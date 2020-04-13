# frozen_string_literal: true

require_relative 'lib/dualcone/version'

Gem::Specification.new do |spec|
  spec.name          = 'dualcone'
  spec.version       = Dualcone::VERSION
  spec.authors       = ['Tom Richards']
  spec.email         = ['tom@tomrichards.net']

  spec.summary       = 'A Ruby source code protection system.'
  spec.description   = 'Dualcone encrypts your Ruby source code.'
  spec.homepage      = 'https://github.com/t-richards/dualcone'
  spec.license       = 'MIT'
  spec.required_ruby_version = Gem::Requirement.new('>= 2.3.0')

  spec.metadata['homepage_uri'] = spec.homepage
  spec.metadata['source_code_uri'] = 'https://github.com/t-richards/dualcone'

  spec.files = Dir.chdir(File.expand_path(__dir__)) do
    `git ls-files -z ext lib`.split("\x0")
  end
  spec.files += Dir.glob('vendor/libhydrogen/{LICENSE,Makefile,README.md}')
  spec.files += Dir.glob('vendor/libhydrogen/*.[c,h]')
  spec.files += Dir.glob('vendor/libhydrogen/impl/**/*')
  spec.files += Dir.glob('vendor/libhydrogen/tests/*.c')
  spec.extra_rdoc_files = %w[README.md LICENSE.txt]
  spec.require_paths = ['lib']
  spec.extensions    = ['ext/dualcone/extconf.rb']

  spec.add_development_dependency 'climate_control'
  spec.add_development_dependency 'irb'
  spec.add_development_dependency 'rake'
  spec.add_development_dependency 'rake-compiler'
  spec.add_development_dependency 'rdoc'
  spec.add_development_dependency 'rspec'
  spec.add_development_dependency 'rubocop'
  spec.add_development_dependency 'warning'
end
