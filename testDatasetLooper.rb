#!/usr/bin/env ruby

require 'pp'
require './DatasetLooper.rb'
looper = DatasetLooper.new
def sep() puts '='*50 end
sep

##############################
puts "DatasetLooper::methods"
sep
puts (looper.methods - Class.methods).sort
puts ""
sep

##############################

puts "DatasetLooper::printLists"
sep
looper.printLists
puts ""
sep

##############################

testnum = ARGV.length==1 ? ARGV[0].to_i : 1
puts "testnum=#{testnum}"
sep

case testnum
# Loops #######
when 0
  looper.datasetListLoop{         |elem| puts "analyze element #{elem}" }
  sep
  looper.datasetListLoopOnlyData{ |elem| puts "analyze element #{elem}" }
  sep
  looper.datasetListLoopOnlyMC{   |elem| puts "analyze element #{elem}" }
  sep
when 1
  looper.subsetListLoop{         |elem| puts "analyze element #{elem}" }
  sep
  looper.subsetListLoopOnlyData{ |elem| puts "analyze element #{elem}" }
  sep
  looper.subsetListLoopOnlyMC{   |elem| puts "analyze element #{elem}" }
  sep
when 2
  looper.allsetListLoop{         |elem| puts "analyze element #{elem}" }
  sep
  looper.allsetListLoopOnlyData{ |elem| puts "analyze element #{elem}" }
  sep
  looper.allsetListLoopOnlyMC{   |elem| puts "analyze element #{elem}" }
  sep
# Pairs #########
when 3
  looper.datasetListPairs{         |pair| puts "analyze pair #{pair}" }
  sep
  looper.datasetListPairsOnlyData{ |pair| puts "analyze pair #{pair}" }
  sep
  looper.datasetListPairsOnlyMC{   |pair| puts "analyze pair #{pair}" }
  sep
when 4
  looper.subsetListPairs{         |pair| puts "analyze pair #{pair}" }
  sep
  looper.subsetListPairsOnlyData{ |pair| puts "analyze pair #{pair}" }
  sep
  looper.subsetListPairsOnlyMC{   |pair| puts "analyze pair #{pair}" }
  sep
when 5
  looper.allsetListPairs{         |pair| puts "analyze pair #{pair}" }
  sep
  looper.allsetListPairsOnlyData{ |pair| puts "analyze pair #{pair}" }
  sep
  looper.allsetListPairsOnlyMC{   |pair| puts "analyze pair #{pair}" }
  sep
# return just the lists by passing no blocks #################
when 6
  pp looper.datasetListLoop
when 7
  pp looper.subsetListLoopOnlyData
when 8
  pp looper.allsetListLoopOnlyMC
when 9
  pp looper.datasetListPairsOnlyMC
#############
else
  $stderr.puts "ERROR: unknown testnum"
end
