#!/usr/bin/env ruby
# loop stringSpinSim.exe -> calcKinematics.exe

require 'fileutils'

# settings #################
NumEventsPerFile = 5e6.to_i
NumFiles         = 14
Modes            = [ 0, 1 ] # stringSpinSim.exe modes
DiskimDir        = 'diskim.sss' # NOTE: files within this directory will be REMOVED
OutrootDir       = 'outroot.sss' # NOTE: files within this directory will be REMOVED
PairType         = '0x34'
NCPUs            = 1  # number of CPUs per node to allocate for slurm
TimeLim          = 48 # time limit [hr]
############################

# check for output dir existence
unless Dir.exist?(DiskimDir) and Dir.exist?(OutrootDir)
  $stderr.puts "ERROR: please create or symlink output directories:"
  [DiskimDir,OutrootDir].each do |dir|
    $stderr.puts " - #{dir}"
  end
  exit 1
end

# clean output dirs
remove_rootfiles = [DiskimDir,OutrootDir].map{ |dir| Dir.glob("#{dir}/*.root") }.flatten
FileUtils.rm remove_rootfiles, verbose: true unless remove_rootfiles.empty?

# start job list file
jobFileName   = "jobs.stringSpinner.slurm"
slurmFileName = jobFileName.gsub /^jobs/, "job"
jobFile       = File.open jobFileName, 'w'

# generate a unique, random seed
PYTHIA_MAX_SEED = 900_000_000 # see pythia param `Random:seed`; cannot generate more than this many jobs
usedSeeds = [0]
generateSeed = Proc.new do |s|
  if usedSeeds.include? s
    generateSeed.call 1 + rand(PYTHIA_MAX_SEED)
  else
    usedSeeds.append s
    s
  end
end

# define job commands
jobFile.puts '#!/bin/bash'
jobFile.puts 'case $1 in'
njobs = 0
NumFiles.times do |filenum|
  Modes.each do |mode|
    uniqueRandomSeed = generateSeed.call 0
    diskimFile = "#{DiskimDir}/mode#{mode}_file#{filenum}_seed#{uniqueRandomSeed}.root"
    cmd = [
      [
        'stringSpinSim.exe',
        NumEventsPerFile,
        mode,
        diskimFile,
        uniqueRandomSeed,
        PairType,
      ].join(' '),
      [
        'calcKinematics.exe',
        diskimFile,
        OutrootDir,
        'mcgen',
      ].join(' ')
    ].join(' && ')
    puts cmd
    jobFile.puts "  #{njobs}) #{cmd} ;;"
    njobs += 1
  end
end
jobFile.puts 'esac'
jobFile.close
system "chmod u+x #{jobFileName}"

# generate slurm file
slurmFile = File.open slurmFileName, 'w'
slurmSet = Proc.new{ |var,val| slurmFile.puts "#SBATCH --#{var}=#{val}" }
slurmFile.puts "#!/bin/bash"
slurmSet.call "job-name",      "stringSpinner"
slurmSet.call "account",       "clas12"
slurmSet.call "partition",     "production"
slurmSet.call "mem-per-cpu",   "#{500/NCPUs}"
slurmSet.call "time",          "#{TimeLim}:00:00"
slurmSet.call "array",         "0-#{njobs-1}"
slurmSet.call "ntasks",        "1"
slurmSet.call "cpus-per-task", "#{NCPUs}"
slurmSet.call "output",        "/farm_out/%u/%x-%A_%a.out"
slurmSet.call "error",         "/farm_out/%u/%x-%A_%a.err"
slurmFile.puts "srun #{jobFileName} $SLURM_ARRAY_TASK_ID"
slurmFile.close()

# execution
puts "-"*30
system "cat #{slurmFileName}"
puts "-"*30
puts "submitting to slurm..."
system "sbatch #{slurmFileName}"
