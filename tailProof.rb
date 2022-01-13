#!/usr/bin/env ruby
# monitor PROOF logs with tail

if ARGV.length!=1
  $stderr.puts "USAGE: #{$0} [bruDir or bruDirGlob]"
  exit 2
end
sandbox = ARGV[0]+"/prooflog"
logdirs = Dir.glob(sandbox+"/*").find_all{|d|d.include?"dispin"}.map{|d|d+"/last-lite-session"}
logfiles = logdirs.map{|d|Dir.glob(d+"/*.log").reject{|f|File.symlink?(f)}}.flatten
system "tail -F #{logfiles.join " "}"
