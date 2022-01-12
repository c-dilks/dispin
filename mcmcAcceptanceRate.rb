#!/usr/bin/env ruby
# get MCMC acceptance rate

if ARGV.length!=1
  $stderr.puts "USAGE: #{$0} [bruDir]"
  exit 2
end
puts "\nMCMC acceptance rates, per PROOF worker:"
sandbox = ARGV[0]+"/prooflog"
logdir = Dir.glob(sandbox+"/*").find{|d|d.include?"dispin"}+"/last-lite-session"
logfiles = Dir.glob(logdir+"/*.log").reject{|f|File.symlink?(f)}
logfiles.each do |logfile|
  res = `grep -i \"acceptance rate\" #{logfile}`
  puts "#{File.basename(logfile)}:\n#{res}\n" if $?.success?
end
