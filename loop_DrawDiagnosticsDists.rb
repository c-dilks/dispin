#!/usr/bin/env ruby
# run DrawDiagnosticsDists.C for a list of files

require './DatasetLooper.rb'
looper = DatasetLooper.new

# run DrawDiagnosticsDists for each file
cmds = []
looper.subsetListLoop do |dataset|
  args = [
    "plots.#{dataset}.root",
    "diagplots/#{dataset}",
  ]
  args.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  cmds << "root -b -q 'DrawDiagnosticsDists.C(#{args.join(',')})'"
end
puts cmds
cmds.each{ |cmd| system cmd } # execute
