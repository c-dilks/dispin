#!/usr/bin/env ruby
# resubmit failed jobs, where the "fail" condition is specified by your specified
# search pattern

require 'awesome_print'
require 'fileutils'

if ARGV.length<1
  puts """
  USAGE: #{$0} [ERR_PATTERN] [CASE_INSENSITIVE]

  - greps for [ERR_PATTERN] in farm_out *.err files, then
    find resubmit associated jobs
    - use errorPrint.sh to show errors

  - if there is a 2nd argument [CASE_INSENSITIVE], grep will
    run with -i option

  """
  exit 2
end
patErr = ARGV[0]
grep = ARGV.length>=2 ? "grep -i" : "grep"

# pretty printer
def pp(str,obj)
  print "#{str} = "
  ap obj
end

# get list of *.err files with [ERR_PATTERN]
grepErr = `errorPrint.sh | #{grep} '#{patErr}'`
puts "grep results:\n#{grepErr}"
errFiles = grepErr.gsub(/:.*$/,"").split(/\n/).uniq
puts "\n#{"="*40}"
pp("error files",errFiles)

# get corresponding job array indices
jobIDs = errFiles.map{ |errFile|
  File.basename(errFile,'.err').split('_').last.to_i
}
pp("job array IDs",jobIDs)

# search joblist, building list of jobs to resubmit
jobListFile = Dir.glob("jobs.*.slurm")
  .grep_v(/\.resubmit\./)
  .max_by{ |f| File.mtime(f) } # main job list
jobList = File.readlines(jobListFile)
resubmitList = []
jobIDs.each{ |jobID|
  if (1..jobList.length)===jobID
    resubmitList << jobList[jobID-1]
  else
    $stderr.puts "ERROR: jobID #{jobID} out of range"
  end
}
pp("\n\nJOBS from #{jobListFile} TO RESUBMIT",resubmitList)

# build job submission script
resubmitListFile = File.open("jobs.resubmit.slurm","w")
resubmitListFile.puts resubmitList
resubmitListFile.close
jobScriptFile = jobListFile.sub(/jobs/,'job')
FileUtils.cp jobScriptFile, 'job.resubmit.slurm'
system "sed -i 's/array.*$/array=1-#{resubmitList.length}/' job.resubmit.slurm"
system "sed -i 's/#{jobListFile}/jobs.resubmit.slurm/' job.resubmit.slurm"
puts '='*40+"\njob.resubmit.slurm\n"+'='*40
system "cat job.resubmit.slurm"
puts "\n"+'='*40+"\njobs.resubmit.slurm\n"+'='*40
system "cat jobs.resubmit.slurm"
puts '='*40

# submit jobs
print "\nDoes this look correct? Have you cross checked with errorPrintFindInLog.rb? [y/N]\n> "
execute = $stdin.gets.chomp=="y"
system "sbatch job.resubmit.slurm" if execute
