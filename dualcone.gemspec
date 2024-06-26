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
  spec.license       = 'ISC'
  spec.required_ruby_version = Gem::Requirement.new('>= 3.1.0')

  spec.metadata['homepage_uri'] = spec.homepage
  spec.metadata['source_code_uri'] = spec.homepage
  spec.metadata['rubygems_mfa_required'] = 'true'

  spec.files = Dir.chdir(File.expand_path(__dir__)) do
    `git ls-files -z ext lib`.split("\x0")
  end
  spec.files += Dir.glob('vendor/libhydrogen/{LICENSE,Makefile,README.md}')
  spec.files += Dir.glob('vendor/libhydrogen/*.[c,h]')
  spec.files += Dir.glob('vendor/libhydrogen/impl/**/*')
  spec.extra_rdoc_files = %w[README.md LICENSE.txt]
  spec.require_paths = ['lib']
  spec.extensions    = ['ext/dualcone/extconf.rb']
end
