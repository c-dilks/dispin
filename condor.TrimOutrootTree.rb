#!/usr/bin/env ruby

if ARGV.length != 1
  $stderr.puts "======================================="
  $stderr.puts "WARNING: condor scripts are deprecated!"
  $stderr.puts "======================================="
  $stderr.puts "USAGE: #{$0} [outroot dir]"
  exit 2
end
outrootFiles = Dir["#{ARGV[0]}/*.root"]
scriptName = File.basename($0).gsub(".rb","")

jobN = "job.#{scriptName}.condor"
File.open(jobN,"w") do |job|
  job.puts "Executable = #{`which root`}"
  job.puts "Universe = vanilla"
  job.puts "notification = never"
  job.puts "getenv = True"
  outrootFiles.each do |outrootFile|
    logFile = "logfiles/#{scriptName}_" + File.basename(outrootFile)
    rootcmd = "TrimOutrootTree.C(\\\"#{outrootFile}\\\")"
    job.puts "Arguments = -b -q #{rootcmd}"
    ["Log","Output","Error"].each{ |l| job.puts "#{l} = #{logFile}.#{l.downcase[0..2]}"}
    job.puts "Queue"
    end
end
system "condor_submit #{jobN}"
