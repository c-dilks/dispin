#!/usr/bin/env ruby
# print errors in PROOF logs that have already been collected in the main
# log files from errorPrintProof.rb

if ARGV.length<1
  $stderr.puts "USAGE: #{$0} [bruDir(s)]"
  $stderr.puts "  globs allowed, no need for quotes"
  exit 2
end
logFiles = ARGV.map{|d|d+"/out*.log"}
system "grep -A1000 'PROOF LOG ERRORS' #{logFiles.join(' ')} | grep --color -E '^.*\.log\-'"
