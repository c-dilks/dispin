#!/usr/bin/env ruby
# loop asymBruFit.C for a variety of arguments
# - main purpose is for asymmetry injection studies
#
# - TODO: make compatible with SLURM (if `on ifarm`, use slurm)
# 

# settings #################
ivString  = "zm"
ivType    = 32
nbins     = [-1, -1, -1]
injSeq    = (46..100).to_a  # Array of injection numbers
minimizer = "minuit"
slurm     = false
############################

jobFileName = "jobs.asymBruFit.#{ivString}.slurm"
jobFile = File.open(jobFileName,"w")


# define asymBruFit.C call, and append to job list
fit = Proc.new do |whichSpinMC|
  bruArgs = [
    "bruspin.#{ivString}.inj#{whichSpinMC}",
    minimizer,
    "",
    ivType,
    *nbins,
    whichSpinMC,
  ]
  bruArgs.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  brufit = "root -b -q $BRUFIT/macros/LoadBru.C"
  cmd = "#{brufit} 'asymBruFit.C(#{bruArgs.join ','})'"
  [$stdout,jobFile].each{ |s| s.puts cmd }
end

# job list generation
jobFile.puts "#!/bin/bash" unless slurm
injSeq.each{ |i| fit.call i } # loop
jobFile.close

# execution
if slurm # run on slurm
  $stderr.puts "ERROR: slurm not yet implemented (TODO)"
  exit 1
else # run locally (sequentially)
  system "chmod u+x #{jobFileName}"
  system "./#{jobFileName}"
end
