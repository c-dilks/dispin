#!/usr/bin/env ruby
# loop asymBruFit.C for a variety of arguments
# - main purpose is for asymmetry injection studies

# settings #################
ivString  = "z"
ivType    = 3
nbins     = [-1, -1, -1]
injSeq    = (0..99).to_a  # Array of injection numbers
minimizer = "minuit"
nCPUs     = 6   # number of CPUs per node to allocate for slurm
############################

# if on ifarm, use slurm; otherwise, run sequentially
slurm = `hostname`.chomp.include? "ifarm"
puts slurm ?
  "MODE: on ifarm, run on CLUSTER with SLURM" :
  "MODE: not on ifarm, run SEQUENTIALLY"

# start job list file
jobFileName = "jobs.asymBruFit.#{minimizer}.#{ivString}.slurm"
slurmFileName = jobFileName.gsub(/^jobs/,"job") if slurm
jobFile = File.open(jobFileName,"w")

# define asymBruFit.C call, and append to job list
fit = Proc.new do |whichSpinMC|
  bruArgs = [
    "bruspin.volatile/bruspin.#{minimizer}.#{ivString}.inj#{whichSpinMC}",
    minimizer,
    "",
    ivType,
    *nbins,
    whichSpinMC,
  ]
  bruArgs.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  brufit = "root -b -q $BRUFIT/macros/LoadBru.C"
  cmd = slurm ?
    "#{brufit} asymBruFit.C(#{bruArgs.join ','})" :
    "#{brufit} 'asymBruFit.C(#{bruArgs.join ','})'"
  [$stdout,jobFile].each{ |s| s.puts cmd }
end

# generate job list
jobFile.puts "#!/bin/bash" unless slurm
injSeq.each{ |i| fit.call i } # loop
jobFile.close

# generate slurm file
if slurm
  slurmFile = File.open(slurmFileName,"w") if slurm
  slurmSet = Proc.new{ |var,val| slurmFile.puts "#SBATCH --#{var}=#{val}" }
  slurmFile.puts "#!/bin/bash"
  slurmSet.call("job-name",      "asymBruFit")
  slurmSet.call("account",       "clas12")
  slurmSet.call("partition",     "production")
  slurmSet.call("mem-per-cpu",   "1200")
  slurmSet.call("time",          "8:00:00")
  slurmSet.call("array",         "1-#{injSeq.length}")
  slurmSet.call("ntasks",        "1")
  slurmSet.call("cpus-per-task", "#{nCPUs}")
  slurmSet.call("output",        "/farm_out/%u/%x-%j-%a.out")
  slurmSet.call("error",         "/farm_out/%u/%x-%j-%a.err")
  slurmFile.puts "srun $(head -n$SLURM_ARRAY_TASK_ID #{jobFileName} | tail -n1)"
  slurmFile.close()
end

# execution
if slurm # run on slurm
  puts "-"*30
  system "cat #{slurmFileName}"
  puts "-"*30
  puts "submitting to slurm..."
  system "sbatch #{slurmFileName}"
else # run locally (sequentially)
  system "chmod u+x #{jobFileName}"
  system "./#{jobFileName}"
end
