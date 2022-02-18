#!/usr/bin/env ruby
# run allsetSymlinker.sh

require './DatasetLooper.rb'
looper = DatasetLooper.new

looper.loopAllsets do |dataset|
  datasetType = dataset.split('.').include?('mc') ? 'mc' : 'data'
  system "singleBinTreeMaker.sh outroot.#{dataset} #{datasetType}"
end
