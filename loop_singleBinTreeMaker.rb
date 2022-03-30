#!/usr/bin/env ruby
# run singleBinTreeMaker.rb for all the datasets

require './DatasetLooper.rb'
require 'colorize'

####### SETTINGS ########
subDir = 'catTrees' # output directory for catTrees (MUST EXIST BEFORE EXECUTION)
#########################

# args
if ARGV.length!=1
  $stderr.puts """
  USAGE #{$0} [DIHADRON]

  DIHADRON can be:"""
  DatasetLooper::Dihadrons.each do |k,v|
    puts "  - #{k.to_s.colorize(:light_red)}: #{v[:title]}" unless k==:none
  end
  exit 2
end

# dataset loopers
dlGeneric  = DatasetLooper.new           # any dihadron
dlSpecific = DatasetLooper.new(ARGV[0])  # specific dihadron
pairType = dlSpecific.pairType

# zip generic and specific loopers
dlGeneric.allsetListLoop.zip(dlSpecific.allsetListLoop) do |genericSet,specificSet|
  datasetType = genericSet.split('.').include?('mc') ? 'mc' : 'data'
  catTreeFile = "#{subDir}/#{DatasetLooper.catTreeBaseName(specificSet)}.root"
  system "singleBinTreeMaker.sh outroot.#{genericSet} #{datasetType} -p #{pairType} -o #{catTreeFile}"
end
