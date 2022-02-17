#!/usr/bin/env ruby
# run allsetSymlinker.sh

require './DatasetLooper.rb'
looper = DatasetLooper.new

looper.loopAllsets do |dataset|
  system "singleBinTreeMaker.sh outroot.#{dataset} data"
end
