#!/usr/bin/env ruby
# run allsetSymlinker.sh

require './DatasetLooper.rb'
looper = DatasetLooper.new

looper.loopAllsets do |dataset|
  system "allsetSymlinker.sh #{dataset.gsub(/\.all$/,'')}"
end
