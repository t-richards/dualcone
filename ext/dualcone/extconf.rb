# frozen_string_literal: true

require 'mkmf'

# :stopdoc:

cflags = %w[
  -march=native -mtune=generic -pipe -fno-plt
  -fstack-protector-strong -fPIC
]

if ENV['DEBUG']
  cflags.unshift('-O0', '-g')
else
  cflags.unshift('-O2')
end

libhydrogen_dir = File.join(__dir__, '..', '..', 'vendor', 'libhydrogen')

abort 'ERROR: make is required to build libhydrogen.' unless find_executable('make')

append_cflags(cflags)

# Build the bundled version of libhydrogen in vendor
Dir.chdir(libhydrogen_dir) do
  system('make clean')
  system("export CFLAGS='#{cflags.join(' ')}'; make")
  system('PREFIX=. make install')

  # Ensure that our bundled version of libhydrogen is always used
  $DEFLIBPATH.unshift("#{libhydrogen_dir}/lib")
  dir_config('hydrogen', "#{libhydrogen_dir}/include", "#{libhydrogen_dir}/lib")
end

abort 'ERROR: Failed to build libhydrogen.' unless have_library('hydrogen') && have_header('hydrogen.h')

create_makefile('dualcone/dualcone')
