#!/usr/bin/env ruby
# run singleBinTreeMaker.sh for all the datasets

require './DatasetLooper.rb'
require 'colorize'

####### SETTINGS ########
subDir = 'catTrees' # output directory for catTrees (MUST EXIST BEFORE EXECUTION)
#########################

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end

# dataset loopers
dlGeneric  = DatasetLooper.new                  # any dihadron
dlSpecific = DatasetLooper.new(ARGV[0].to_sym)  # specific dihadron
pairType = dlSpecific.pairType

# zip generic and specific loopers
dlGeneric.allsetListLoop.zip(dlSpecific.allsetListLoop) do |genericSet,specificSet|
  next if genericSet.split('.').include?('bibending')
  datasetType = genericSet.split('.').include?('mc') ? 'mc' : 'data'
  catTreeFile = "#{subDir}/#{DatasetLooper.catTreeBaseName(specificSet)}.root"
  system "singleBinTreeMaker.sh outroot.#{genericSet} #{datasetType} -p #{pairType} -o #{catTreeFile}"
end
