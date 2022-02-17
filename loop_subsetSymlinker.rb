#!/usr/bin/env ruby
# run subsetSymlinker.sh

require './DatasetLooper.rb'
looper = DatasetLooper.new

numFiles = 8 # number of files in each subset

looper.loopDatasets do |dataset|
  system "subsetSymlinker.sh outroot.#{dataset} #{numFiles}"
end
