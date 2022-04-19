#!/usr/bin/env ruby
# run all steps to calculate the systematic uncertainty from baryon decay

require './DatasetLooper.rb'
require 'awesome_print'
require 'thread/pool'
require 'pry'

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
dihadronSym = ARGV[0].to_sym
dl = DatasetLooper.new(dihadronSym)

# settings #################
mcSets  = dl.allsetListLoopOnlyMC.reject{|f|f.include?'bibending'} # do not include bibending
ivTypes = dl.binHash.keys#.select{|i|i==2}
limiter = 3_000_000 # this many events, per dataset (set to zero to take all)
############################


# build commands
step1cmds = []
step2cmds = []
mcSets.product(ivTypes).each do |mcSet,ivType|

  # set treeFile name
  ivName   = dl.binHash[ivType][:name]
  treeFile = "baryonTrees/#{mcSet}.#{ivName}.root"

  # remove dihadorn pair name from `mcSet`
  mcAllSet = mcSet
    .split('.')
    .reject{ |m| m==dihadronSym.to_s }
    .join('.')

  # step 1 command: systematicBaryonDecay1.exe
  step1cmds << [
    "systematicBaryonDecay1.exe",
    "-d outroot.#{mcAllSet}",
    "-p #{dl.pairType}",
    "-i #{ivType}",
    "-n #{dl.binHash[ivType][:bins].join(' ')}",
    "-o #{treeFile}",
    "-l #{limiter}",
    ]

  # step 2 command: systematicBaryonDecay2.rb
  step2cmds << [
    "systematicBaryonDecay2.rb",
    treeFile,
  ]

end
ap step1cmds
ap step2cmds


# execution
pool = Thread.pool(`nproc`.to_i-2)
step1cmds.zip(step2cmds) do |step1cmd,step2cmd|
  step1 = step1cmd.join(' ')
  step2 = step2cmd.join(' ')
  ### single-threaded
  # system step1
  # system step2
  ### multi-threaded
  pool.process do
    system step1
    system step2
  end
end
pool.shutdown
