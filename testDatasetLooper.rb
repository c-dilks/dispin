#!/usr/bin/env ruby

require './DatasetLooper.rb'
looper = DatasetLooper.new
def sep() puts '='*50 end

looper.printLists

sep
sep
looper.datasetListLoop{ |elem| puts "analyze element #{elem}" }
sep
looper.subsetListLoop{  |elem| puts "analyze element #{elem}" }
sep
looper.allsetListLoop{  |elem| puts "analyze element #{elem}" }

sep
sep
looper.datasetListPairs{ |pair| puts "analyze pair #{pair}" }
sep
looper.subsetListPairs{  |pair| puts "analyze pair #{pair}" }
sep
looper.allsetListPairs{  |pair| puts "analyze pair #{pair}" }

