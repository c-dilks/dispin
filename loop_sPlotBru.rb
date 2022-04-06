#!/usr/bin/env ruby
# run sPlotBru.C with arguments for the final fit

require './DatasetLooper.rb'
require 'awesome_print'

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
dl = DatasetLooper.new(ARGV[0].to_sym)

# settings ###############################################
idString   = "apr4" # output ID
subDirIn   = "catTrees"
subDirOut  = "splots"
datasets   = dl.allsetListLoopOnlyData#.select{ |dataset| dataset.include?'bibending' }
binSchemes = DatasetLooper::BinHash
##########################################################

# add single bin scheme to BinHash; this is useful for `DrawSweightedDists.C` for 
# the comparison of sWeighted data to MC
binSchemes[0] = { :bins=>[1], :name=>'singlebin', :xTitle=>'single bin', :xTranslation=>0.0 }

# loop over all possible settings, defining sPlotBru.C calls
cmds = []
ivTypes  = binSchemes.keys
datasets.product(ivTypes).each do |dataset,ivType|

  # sPlotBru.C arguments
  bruArgs = [
    "#{subDirIn}/catTreeData.#{dataset}.idx.trimmed.root",
    "#{subDirOut}/#{idString}.#{dataset}.#{binSchemes[ivType][:name]}",
    dl.pairType,
    ivType>0 ? ivType : 1,
    *binSchemes[ivType][:bins],
  ]
  bruArgs.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  ap bruArgs
  brufit = "root -b -q $BRUFIT/macros/LoadBru.C"
  cmd = "#{brufit} 'sPlotBru.C(#{bruArgs.join ','})'"
  cmds << cmd
end

# execution
cmds.each do |cmd| system cmd end
