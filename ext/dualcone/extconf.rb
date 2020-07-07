# frozen_string_literal: true

require 'mkmf'

cflags = %w[
  -march=native -fno-exceptions -pipe
  -fstack-protector-strong -fPIC -Wall -Werror
  -Wno-missing-braces
]

if ENV['DEBUG']
  cflags.unshift('-O0', '-g')
else
  cflags.unshift('-Os')
end

LIBHYDROGEN_DIR = File.join(__dir__, '..', '..', 'vendor', 'libhydrogen')

abort 'ERROR: make is required to build libhydrogen.' unless find_executable('make')

append_cflags(cflags)

# Build the bundled version of libhydrogen in vendor
Dir.chdir(LIBHYDROGEN_DIR) do
  system('make clean')
  system("export CFLAGS='#{cflags.join(' ')}'; make")
  system('PREFIX=. make install')

  # Ensure that our bundled version of libhydrogen is always used
  $DEFLIBPATH.unshift("#{LIBHYDROGEN_DIR}/lib")
  dir_config('hydrogen', "#{LIBHYDROGEN_DIR}/include", "#{LIBHYDROGEN_DIR}/lib")
end

abort 'ERROR: Failed to build libhydrogen.' unless have_library('hydrogen') && have_header('hydrogen.h')

create_makefile('dualcone/dualcone')
