#!/usr/bin/env ruby
# run DrawSweightedDists.C with arguments for the final fit

require './DatasetLooper.rb'

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
dl = DatasetLooper.new(ARGV[0].to_sym)

# settings ###############################################
idString   = "apr4" # output ID
datasets   = dl.allsetListLoopOnlyData#.select{ |dataset| dataset.include?'bibending' }
##########################################################

# loop over all datasets
cmds = []
datasets.each do |dataset|

  # find matching MC catTree
  datasetMC = dl.matchByTorus(dataset,dl.allsetListLoopOnlyMC)
  catTreeMC = "#{DatasetLooper.catTreeBaseName(datasetMC)}.idx.trimmed.root"

  # macro arguments
  args = [
    "#{DatasetLooper::SplotDir}/#{idString}.#{dataset}.singlebin", # only comparing single bin sPlots
    "#{catTreeMC}",
  ]
  args.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  cmds << "root -b -q 'DrawSweightedDists.C(#{args.join ','})'"

end

# execution
cmds.each do |cmd| system cmd end
