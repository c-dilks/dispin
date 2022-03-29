#!/usr/bin/env ruby
# monitor PROOF logs with tail

if ARGV.length<1
  $stderr.puts "USAGE: #{$0} [farmout bruDir(s)]"
  $stderr.puts "  globs allowed, no need for quotes"
  exit 2
end
sandboxList = ARGV.map{|d|d+"/prooflog"}
logDirList = sandboxList.map do |sandbox|
  Dir.glob(sandbox+"/*").find_all{|d|d.include?"dispin"}.map{|d|d+"/last-lite-session"}
end.flatten
logfiles = logDirList.map{|d|Dir.glob(d+"/*.log").reject{|f|File.symlink?(f)}}.flatten
system "tail -F #{logfiles.join " "}"
