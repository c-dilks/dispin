#!/usr/bin/env ruby
# run singleBinTreeMaker.sh for all the datasets

require './DatasetLooper.rb'
require 'awesome_print'

####### SETTINGS ########
subDir = 'catTrees' # output directory for catTrees (MUST EXIST BEFORE EXECUTION)
# CAUTION: if DatasetLooper.rb has been updated, this script may not work correctly;
#          check the singleBinTreeMaker.sh arguments before execution
#########################

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end

# dataset loopers
dl = {
  :generic  => DatasetLooper.new,                 # any dihadron
  :specific => DatasetLooper.new(ARGV[0].to_sym), # specific dihadron
}
pairType = dl[:specific].pairType

# get allset lists, applying a filter, and sort
allsetList = dl.map do |k,dlObj|
  list = dlObj.allsetListLoop
    .reject{ |d| d.include?'bibending' }
    .sort
  [k,list]
end.to_h
puts "ALLSET LISTS = "
ap allsetList

# zip generic and specific loopers
allsetList[:generic].zip(allsetList[:specific]) do |genericSet,specificSet|
  ap [genericSet,specificSet]
  datasetType = genericSet.split('.').include?('mc') ? 'mc' : 'data'
  catTreeFile = "#{subDir}/#{DatasetLooper.catTreeBaseName(specificSet)}.root"
  system "singleBinTreeMaker.sh outroot.#{genericSet} #{datasetType} -p #{pairType} -o #{catTreeFile}"
end
