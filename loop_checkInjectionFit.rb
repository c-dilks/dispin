#!/usr/bin/env ruby
# run checkInjectionFit.C in a loop, for all given injection models

# arguments
if ARGV.length < 1
  $stderr.puts """
  USAGE: #{$0} [bruspin dir, with \"ITER\"]
    [bruspin dir]: fit results directory
    [bruspin dir] must have a substring, to indicate injection number, either:
    - \"inj0\", denoting the 0th injection (replaced with inj1, inj2, ...)
    - \"ITER\", denoting the injection number (replaced with 1, 2, ...)
  """
  exit 2
end
bruDir = ARGV[0]

if bruDir.include? "inj0"
  bruDir = bruDir.gsub("inj0","inj*")
elsif bruDir.include? "ITER"
  bruDir = bruDir.gsub("ITER","*")
else
  $stderr.puts "ERROR: bruspin dir does not contain iterator substring"
  exit 1
end

bruDirList = Dir.glob(bruDir)
puts bruDirList
