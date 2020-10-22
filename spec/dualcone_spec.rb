# frozen_string_literal: true

require 'spec_helper'

RSpec.describe Dualcone do
  let(:key) do
    'd0516606a61d10aeef791dedb0b298c0b52114d377ca23ef5591506460a90111'
  end
  let(:fake_header) { SecureRandom.hex(36) }

  context '.run' do
    context 'given a non-string argument' do
      args = [
        nil,
        Object.new,
        Class.new,
        Module.new,
        1.0,
        /regexp/,
        [],
        {},
        Struct.new(:foo),
        1,
        true,
        false,
        :symbol
      ]

      args.each do |arg|
        it "raises a TypeError for #{arg}" do
          expect do
            described_class.run(arg)
          end.to raise_error(TypeError, /expected String/)
        end
      end
    end

    context 'without an encryption key' do
      it 'raises a KeyError' do
        code = ''

        expect do
          described_class.run(code)
        end.to raise_error(KeyError, /environment variable not found/)
      end
    end

    context 'without valid hex-encoded data' do
      it 'raises a fatal error' do
        code = "#{fake_header}zz"

        ClimateControl.modify DUALCONE_HEX_KEY: key do
          expect do
            described_class.run(code)
          end.to raise_error(/unable to hex-decode ruby code/)
        end
      end
    end

    context 'without a valid encryption header' do
      it 'raises a fatal error' do
        code = 'aa'

        ClimateControl.modify DUALCONE_HEX_KEY: key do
          expect do
            described_class.run(code)
          end.to raise_error(/unable to run code: too short/)
        end
      end
    end

    context 'without a valid encrypted message' do
      it 'raises a fatal error' do
        code = "#{SecureRandom.hex(64)}aa"

        ClimateControl.modify DUALCONE_HEX_KEY: key do
          expect do
            described_class.run(code)
          end.to raise_error(/unable to decrypt ruby code/)
        end
      end
    end

    context 'with a valid key and code' do
      let(:code) do
        <<~CODE.strip
          517a432610f6e0a0be2dabb461dd82cc5100428e59b067a72c6722537ab1488d7529740dd11652517da0b1ea1725dd0208530d9aea2f
        CODE
      end

      it 'evaluates the code' do
        ClimateControl.modify DUALCONE_HEX_KEY: key do
          described_class.run(code)
        end

        expect(TEST_SUCCESS).to eq(true)
      end

      context 'multiple runs to check for uninitialized memory' do
        let(:other_code) do
          <<~CODE.strip
            ca3cba5d065e37c8e2eaeb95c6000b9faaf15dcc24f6ca6c81ed62e73fd8cced21a08b336e11bbb7ce9d088add8c4edf0c2663
          CODE
        end

        64.times do |i|
          it "evaluates the code (try #{i})" do
            ClimateControl.modify DUALCONE_HEX_KEY: key do
              described_class.run(other_code)
            end
          end
        end
      end
    end
  end

  context '.encrypt' do
    context 'without an encryption key' do
      it 'raises a KeyError' do
        file = Tempfile.new

        expect do
          described_class.encrypt(file.path)
        end.to raise_error(KeyError, /environment variable not found/)
      end
    end

    context 'with an encryption key' do
      it 'returns nil' do
        file = Tempfile.new

        ClimateControl.modify DUALCONE_HEX_KEY: key do
          result = described_class.encrypt(file.path)

          expect(result).to eq(nil)
        end
      end

      it 'modifies the file' do
        file = Tempfile.new

        ClimateControl.modify DUALCONE_HEX_KEY: key do
          described_class.encrypt(file.path)
          result = File.read(file.path)

          expect(result).to start_with("require 'dualcone'")
          expect(result).to include("Dualcone.run('")
          expect(result).to end_with("')\n")
        end
      end

      it 'generates code without null bytes' do
        file = Tempfile.new

        ClimateControl.modify DUALCONE_HEX_KEY: key do
          described_class.encrypt(file.path)
          result = File.read(file.path)

          expect(result).to_not include("\u0000")
        end
      end
    end
  end

  context '.generate_key' do
    it 'generates a key' do
      result = described_class.generate_key

      expect(result).to be_a(String)
      expect(result.length).to eq(64)
    end
  end
end
