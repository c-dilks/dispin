#!/usr/bin/env ruby
# run asymBruFit.C with arguments for the final fit

# settings #################
idString    = "final.feb3"
datasets    = ["rga", "rgb"]
minimizers  = ["minuit", "mcmccov"]
ivBinnings  = { # map `ivType` to number of bins for each dimension
  1  => [12],
  2  => [12],
  32 => [4,3],
  42 => [4,3],
}
timeLim = 36 # time limit [hr]
############################

sep = Proc.new{ |title| puts "\n#{title}\n"+"="*40 }

# if on ifarm, use slurm; otherwise, run sequentially
sep.call "job settings"
slurm = `hostname`.chomp.include? "ifarm"
puts slurm ?
  "Mode: on ifarm, run on cluster with Slurm" :
  "Mode: not on ifarm, run sequentially"

# determine number of CPUs to allocate per slurm node
numBins = ivBinnings.values.map{ |nbins| nbins.inject :* }
nCPUs = numBins.max # use max number of bins from any job -> could waste resources if some jobs have less bins
# nCPUs = numBins.min # use min number of bins from any job -> jobs with more bins will take longer
if slurm
  puts "Requesting #{nCPUs} CPUs per slurm node"
  $stderr.puts "\nWARNING: some jobs have more/less bins than others, impacting resource usage (see #{$0})\n\n" if numBins.uniq.length>1
end

# start job list file
jobFileName = "jobs.#{idString}.slurm"
slurmFileName = jobFileName.gsub(/^jobs/,"job") if slurm
jobFile = File.open(jobFileName,"w")
jobFile.puts "#!/bin/bash" unless slurm

# reformat some settings
ivTypes = ivBinnings.keys
ivBinnings.each{ |_,nbins| nbins.replace (nbins+[-1,-1])[0..2] } # fill rest of nbins Arrays with default `-1` values

# loop over all possible settings, defining asymBruFit.C calls, and generate job list
# - ASSUME there will be one job per possible setting
sep.call "job list"
settings = datasets.product(ivTypes,minimizers).each do |dataset,ivType,minimizer|
  bruDir = "bruspin.volatile/" + [idString,dataset,ivType,minimizer].join('.')
  bruArgs = [
    bruDir,
    minimizer,
    "",
    ivType,
    *ivBinnings[ivType],
  ]
  bruArgs.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  brufit = "root -b -q $BRUFIT/macros/LoadBru.C"
  cmd = slurm ?
    "#{brufit} asymBruFit.C(#{bruArgs.join ','})" :
    "#{brufit} 'asymBruFit.C(#{bruArgs.join ','})'"
  [$stdout,jobFile].each{ |s| s.puts cmd }
end
jobFile.close

# generate slurm file
if slurm
  slurmFile = File.open(slurmFileName,"w") if slurm
  slurmSet = Proc.new{ |var,val| slurmFile.puts "#SBATCH --#{var}=#{val}" }
  slurmFile.puts "#!/bin/bash"
  slurmSet.call("job-name",      "#{idString}")
  slurmSet.call("account",       "clas12")
  slurmSet.call("partition",     "production")
  slurmSet.call("mem-per-cpu",   "#{7200/nCPUs}")
  slurmSet.call("time",          "#{timeLim}:00:00")
  slurmSet.call("array",         "1-#{settings.length}")
  slurmSet.call("ntasks",        "1")
  slurmSet.call("cpus-per-task", "#{nCPUs}")
  slurmSet.call("output",        "/farm_out/%u/%x-%A_%a.out")
  slurmSet.call("error",         "/farm_out/%u/%x-%A_%a.err")
  slurmFile.puts "srun $(head -n$SLURM_ARRAY_TASK_ID #{jobFileName} | tail -n1)"
  slurmFile.close()
end

# execution
if slurm # run on slurm
  sep.call "slurm script"
  system "cat #{slurmFileName}"
  sep.call "launch"
  system "sbatch #{slurmFileName}"
else # run locally (sequentially)
  sep.call "launch"
  system "chmod u+x #{jobFileName}"
  system "./#{jobFileName}"
end
