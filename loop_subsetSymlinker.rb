#!/usr/bin/env ruby
# run subsetSymlinker.sh

require './DatasetLooper.rb'
if ARGV.length<1
  $stderr.puts "USAGE: #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
Dihadron = ARGV[0]
looper = DatasetLooper.new(Dihadron.to_sym)

numFiles = 8 # number of files in each subset

looper.datasetListLoop do |dataset|
  dataset.sub!("#{Dihadron}.",'')
  system "subsetSymlinker.sh outroot.#{dataset} #{numFiles}"
end
