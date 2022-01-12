#!/usr/bin/env ruby
# print errors in PROOF logs

if ARGV.length!=1
  $stderr.puts "USAGE: #{$0} [bruDir]"
  exit 2
end
puts "\nPROOF LOG ERRORS:"
sandbox = ARGV[0]+"/prooflog"
logdir = Dir.glob(sandbox+"/*").find{|d|d.include?"dispin"}+"/last-lite-session"
logfiles = Dir.glob(logdir+"/*.log").reject{|f|File.symlink?(f)}.sort
logfiles.each do |logfile|
  cmd = "grep -i error #{logfile}"
  cmd += "|grep -v MATRIX"
  cmd += "|grep -v VALUE"
  cmd += "|grep -v \"Floating Parameter\""
  cmd += "|grep -v \"retrieving message from input socket\""
  res = `#{cmd}`
  puts "\n------->>> #{File.basename(logfile)}:\n#{res}" if $?.success?
end
