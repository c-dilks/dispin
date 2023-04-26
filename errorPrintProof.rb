#!/usr/bin/env ruby
# print errors in PROOF logs

if ARGV.length<1
  $stderr.puts "USAGE: #{$0} [farmout bruDir(s)]"
  $stderr.puts "  globs allowed, no need for quotes"
  exit 2
end
puts "\nPROOF LOG ERRORS:"
sandboxList = ARGV.map{|d|d+"/prooflog"}
logDirList = sandboxList.map do |sandbox|
  Dir.glob(sandbox+"/*").find_all{|d|d.include?"dispin"}.map{|d|d+"/last-lite-session"}
end.flatten
logfiles = logDirList.map{|d|Dir.glob(d+"/*.log").reject{|f|File.symlink?(f)}}.flatten.sort
logfiles.each do |logfile|
  cmd = "grep -iE 'error|did NOT' #{logfile}"
  cmd += "|grep -v MATRIX"
  cmd += "|grep -v VALUE"
  cmd += "|grep -v \"Floating Parameter\""
  cmd += "|grep -v \"retrieving message from input socket\""
  cmd += "|grep -v \"plotOn: WARNING: variable is not an explicit dependent:\""
  cmd += "|grep -v \" is not a dependent and will be ignored.\""
  cmd += "|grep -v \"cache/libRooStats_rdict.pcm file does not exist\""
  cmd += "|grep -v \"out of this region. Error log follows.\""
  cmd += "|grep -vi \"error matrix\""
  cmd += "|grep -v \"Minimized function has error status.\""
  res = `#{cmd}`
  puts "\n------->>> #{logfile}:\n#{res}" if $?.success?
end
