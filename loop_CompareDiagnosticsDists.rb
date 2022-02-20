#!/usr/bin/env ruby
# run CompareDiagnosticsDists.C for various pairs of files

require './DatasetLooper.rb'
looper = DatasetLooper.new

# run CompareDiagnosticsDists, for each pair of datasets
cmds = []
looper.subsetListPairs do |pair|

  # convert dataset names to titles
  pairTitles = pair.map{|set| looper.datasetTitle(set) }

  # root arguments
  args = [
    *pair.map{ |dataset| "plots.#{dataset}.root" },
    "diagcomps/#{pair.join("__vs__")}",
    *pairTitles,
  ]
  args.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  cmds << "root -b -q 'CompareDiagnosticsDists.C(#{args.join(',')})'"

end
puts cmds
cmds.each{ |cmd| system cmd } # execute
