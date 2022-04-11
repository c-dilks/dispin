#!/usr/bin/env ruby
# get yields in each catTree

require 'RubyROOT'
include Root

Dir.glob("catTrees/catTree*.idx.trimmed.root").each do |treeFileN|
  TFile.open(treeFileN,"READ") do |treeFile|
    puts "#{treeFileN} #{treeFile.Get('tree').auto_cast.GetEntries}"
  end
end
