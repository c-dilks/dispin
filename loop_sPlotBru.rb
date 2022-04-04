#!/usr/bin/env ruby
# run sPlotBru.C with arguments for the final fit

require './DatasetLooper.rb'

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
dl = DatasetLooper.new(ARGV[0])

# settings ###############################################
idString = "apr4" # output ID
subDirIn = "catTrees"
subDirOut = "splots"
datasets = dl.allsetListLoopOnlyData#.select{ |dataset| dataset.include?'bibending' }
ivTypes  = DatasetLooper::BinHash.keys
##########################################################

# loop over all possible settings, defining sPlotBru.C calls
cmds = []
datasets.product(ivTypes).each do |dataset,ivType|

  # sPlotBru.C arguments
  bruArgs = [
    "#{subDirIn}/catTreeData.#{dataset}.idx.trimmed.root",
    "#{subDirOut}/#{idString}.#{dataset}.#{DatasetLooper::BinHash[ivType][:name]}",
    ivType,
    *DatasetLooper::BinHash[ivType][:bins],
  ]
  bruArgs.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  brufit = "root -b -q $BRUFIT/macros/LoadBru.C"
  cmds << "#{brufit} 'sPlotBru.C(#{bruArgs.join ','})'"
end

# execution
cmds.each do |cmd| system cmd end
