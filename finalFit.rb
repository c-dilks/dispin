#!/usr/bin/env ruby
# run asymBruFit.C with arguments for the final fit

# settings #################
idString  = "test"
datasets = ["rga", "rgb"]
ivBinning = { # map `ivType` to number of bins for each dimension
  1  => [6],
  2  => [6],
  32 => [4,3],
  42 => [4,3],
}
minimizers = ["minuit", "mcmccov"]
nCPUs     = 6   # number of CPUs per node to allocate for slurm # TODO: determine from number of bins
timeLim   = 48 # time limit [hr] # TODO: minimizer.contains? "mcmc" ? 48 : 5
############################

# if on ifarm, use slurm; otherwise, run sequentially
slurm = `hostname`.chomp.include? "ifarm"
puts slurm ?
  "MODE: on ifarm, run on CLUSTER with SLURM" :
  "MODE: not on ifarm, run SEQUENTIALLY"

# make brudir names
brudirs = datasets.map{ |d| [idString,dataset,minimizer,iv].join "." } # AQUI: make this a proc?

# argsets: list of sets of arguments; each "argset" will be one job
sep = Proc.new{ |title| puts "\n#{title}\n"+"="*40 }
argsets = datasets.product( ivBinning.keys, minimizers )
sep.call "ARGSETS"
argsets.each{|a| puts "#{a}"}
exit



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
  slurmSet.call("mem-per-cpu",   "#{7200/nCPUs}")
  slurmSet.call("time",          "#{timeLim}:00:00")
  slurmSet.call("array",         "1-#{injSeq.length}")
  slurmSet.call("ntasks",        "1")
  slurmSet.call("cpus-per-task", "#{nCPUs}")
  slurmSet.call("output",        "/farm_out/%u/%x-%A_%a.out")
  slurmSet.call("error",         "/farm_out/%u/%x-%A_%a.err")
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
