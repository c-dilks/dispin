#!/usr/bin/env ruby

require './DatasetLooper.rb'

looper = DatasetLooper.new

# using DatasetLooper::Filters
# puts looper.allsetList.onlyMC

looper.loopallsetList do |d|
  puts "#{d} is working"
end
