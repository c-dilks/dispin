#!/usr/bin/env ruby
# truncate a tree, resulting a new tree with `fraction*originalTree->GetEntries()` entries

require 'RubyROOT'
include Root

if ARGV.length!=2
  $stderr.puts """
  USAGE: #{$0} [tree] [fraction]
  """
  exit 2
end
inFileN = ARGV[0]
fraction = ARGV[1].to_f

# check fraction \in [0,1]
unless (0..1)===fraction
  $stderr.puts "ERROR: fraction must be in [0,1]"
  exit 1
end

# truncation
outFileN = inFileN.sub(/\.root$/,'.TRUNCATED.root')
puts "outFileN=#{outFileN}"
TFile.open(inFileN,"READ") do |inFile|
  TFile.open(outFileN,"RECREATE") do |outFile|
    inTree = inFile.Get('tree').auto_cast
    entries = inTree.GetEntries
    truncatedEntries = (fraction * inTree.GetEntries).round
    puts "original tree entries  = #{entries}"
    puts "truncated tree entries = #{truncatedEntries}"
    outTree = inTree.CloneTree(truncatedEntries)
    outTree.Write
  end
end
puts "\n - PRODUCED  #{outFileN}"
