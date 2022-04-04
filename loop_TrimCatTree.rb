#!/usr/bin/env ruby
# run TrimCatTree.C for all the datasets

require './DatasetLooper.rb'

####### SETTINGS ########
subDir = 'catTrees'
#########################

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
dl = DatasetLooper.new(ARGV[0])

# loop over allsets
dl.allsetListLoop.each do |dataset|
  catTreeFile = "#{subDir}/#{DatasetLooper.catTreeBaseName(dataset)}.idx.root"
  system "root -b -q 'TrimCatTree.C(\"#{catTreeFile}\")'"
end
