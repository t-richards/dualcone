# Dualcone

[![CircleCI](https://circleci.com/gh/t-richards/dualcone.svg?style=svg)](https://circleci.com/gh/t-richards/dualcone)

Dualcone is a Ruby source code protection system. Dualcone uses symmetric encryption to protect your source code.

Dualcone is a self-contained gem. It brings along its own copy of the lightweight cryptographic library, [libhydrogen][libhydrogen].

Dualcone supports GNU + Linux and other Unix-like operating systems. Windows is not supported.

## Roadmap

### Part 1
 - [x] Key generation: `Dualcone.generate_key`
 - [x] Encrypted code running: `Dualcone.run(code)`
 - [x] Encrypted code generation: `Dualcone.encrypt(path)`
 - [x] Specs passing

### Part 2
 - [x] Runnable trivial ruby script
 - [ ] Runnable non-trivial ruby script
 - [ ] Runnable sinatra app
 - [ ] Runnable rails app

## Installation

Add this gem to your application's Gemfile:

```ruby
gem 'dualcone'
```

And then execute:

```bash
$ bundle install
```

Or install it yourself as:

```bash
$ gem install dualcone
```

You need to have a C compiler and `make` installed on your system to be able to build this gem's native code and the included version of `libhydrogen`.

## Usage

1. Generate a secret encryption key.

    ```ruby
    require 'dualcone'
    Dualcone.generate_key
    => "764888c92f3059c88524225b622cd178856877cf3537230a9d7f5b5b6d8850c5"
    ```

2. Place your encryption key in the `DUALCONE_HEX_KEY` environment variable:

    ```bash
    $ export DUALCONE_HEX_KEY="8c8f91f84d8e554dc03277ce2f038af95cd932e2b65011969e77d3ac18d7bdd9"
    ```

    This environment variable is required for both encrypting files as well as running already-encrypted files.

3. Encrypt your Ruby source code file(s).

    :warning: Your source code file will be modified in-place. This is a one-way operation! :warning:

    For example, let's say we have a file named `hello.rb` with the following contents:

    ```ruby
    puts 'Hello, world!'
    ```

    You can encrypt this file using `Dualcone.encrypt(path)`

    ```ruby
    Dualcone.encrypt('hello.rb')
    ```

    The entire contents of the file `hello.rb` will be replaced with a call to `Dualcone.run(code)`:

    ```ruby
    require 'dualcone'
    Dualcone.run('7f1a1b6a047aee2403e415044b72f2ac2997cef689960df46afdfe6d7c657e18dbae1bea3bbe33ae9157cb2f7b22f34db69b2eb41e05aa512151')
    ```

4. Finally, test your encrypted code by running it.

    ```bash
    $ ruby hello.rb
    ```

## Development

1. `git clone git@github.com:t-richards/dualcone.git` to clone the repo.
2. `bin/setup` to install dependencies and fetch git submodules.
3. `bin/rake compile` to build the gem's native extensions.
4. `bin/rspec` to run the tests.
5. `bin/rubocop` to check code style.

You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bin/rake install`. To release a new version, update the version number in `version.rb`, and then run `bin/rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org][rubygems].

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/t-richards/dualcone.

## License

The gem is available as open source under the terms of the [MIT License][mit-license].

[libhydrogen]: https://github.com/jedisct1/libhydrogen
[mit-license]: https://opensource.org/licenses/MIT
[rubygems]: https://rubygems.org
